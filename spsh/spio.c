#include "spio.h"

#define MAX_FILE_IN_USED_COUNT 1

static struct {
    SP_FILE files[MAX_FILE_IN_USED_COUNT];
    int open_file_count;
} file_control;

int existed(int type, char *filename) {
    int cdn = get_shell_env()->curr_dir_num;
    return spfs_existed(&(get_shell_env()->fs_sys_blk), 
        type, &cdn, filename);
}

int open(const char *filename, int mode) {
    int cdn = get_shell_env()->curr_dir_num;
    int ret = -1;
    int ifile = 0;
    if (file_control.open_file_count < MAX_FILE_IN_USED_COUNT) { 
        ifile = spfs_open(&(get_shell_env()->fs_sys_blk), 
            cdn, (char *)filename);
        if (ifile) {
            ret = file_control.open_file_count;
            file_control.files[ret].is_open = 1;
            file_control.files[ret].mode = mode;
            file_control.files[ret].file = ifile;
            ++file_control.open_file_count;
        }
    }
    return ret + 1;
}

int write(int file, char data[], int size) {
    int cdn = get_shell_env()->curr_dir_num;
    int wc = 0;
    if (file && file_control.files[file-1].is_open) {
        --file;
        if (file_control.files[file].mode == FILE_OPEN_WRITE) {
            wc = SPFS_WRITE_REWRITE;
        }
        else if (file_control.files[file].mode == FILE_OPEN_APPEND) {
            wc = SPFS_WRITE_APPEND;
        }
        wc = spfs_write(&(get_shell_env()->fs_sys_blk), 
                file_control.files[file].file, data, size, wc);
        get_shell_env()->ch2dir(cdn); /**< 更新 shell 目录 */
    }
    return wc;
}

int read(int file, int offset, char data[], int size) {
    int cdn = get_shell_env()->curr_dir_num;
    if (file && file_control.files[file-1].is_open &&
        file_control.files[file-1].mode == FILE_OPEN_READ) {
        --file;
        return spfs_read(&(get_shell_env()->fs_sys_blk), 
            file_control.files[file].file, offset, data, size);
    }
    return 0;
}

void close(int file) {
    if (file_control.open_file_count > 0) {
        --file_control.open_file_count;
        --file;
        file_control.files[file].is_open = 0;
        file_control.files[file].mode = 0;
        file_control.files[file].file = 0;
    }
}
