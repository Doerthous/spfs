#include "spfs.h"


static void set_bit(void *bitmap, int length, int i) {
    char *p = (char *)bitmap;
    const int byte = sizeof(char) * 8;
    if (i < length) {
        p[i / byte] |= (1 << (i % byte));
    }
}
static void clear_bit(void *bitmap, int length, int i) {
    char *p = (char *)bitmap;
    const int byte = sizeof(char) * 8;
    if (i < length) {
        p[i / byte] &= ~(1 << (i % byte));
    }
}
static int test_bit(void *bitmap, int length, int i) {
    char *p = (char *)bitmap;
    const int byte = sizeof(char) * 8;
    if (i < length) {
        int byte = sizeof(char) * 8;
        return p[i / byte] & (1 << (i % byte));
    }
    return 0;
}



#define DIRECTORY_SIZE              sizeof(spfs_directory)

static char block_buffer[MAX_BLOCK_SIZE];
static spfs_file file_buffer;

#define read_block(device, bn, buf) rw_block(device, bn, buf, read_sector)
#define write_block(device, bn, buf) rw_block(device, bn, buf, write_sector)

static void rw_block(int device, int bn, char *buf, int (*rw_sector)(int, int, char *, int)) {
    spfs_parameter sb;
    int sb_sec = 0;
    read_sector(device, 1, (char *)&sb, sizeof(sb));
    if (strncmp(sb.signature, "spfs", 4)) {
        read_sector(device, 2, (char *)&sb, sizeof(sb));
        sb_sec = 1;
    }
    if (bn > 0) { // 
        rw_sector(device, bn*sb.sector_count_per_block+sb_sec, buf, get_sector_size(device));
    }
}


void get_system_block(int device, spfs_parameter *sb) {
    read_block(device, 1, block_buffer);
    sb->device = device;
    memcpy(sb, block_buffer, sizeof(spfs_parameter));
}
void set_system_block(spfs_parameter *sb) {
    memcpy(block_buffer, sb, sizeof(spfs_parameter));
    write_block(sb->device, 1, block_buffer);
}
void get_directory_map_block(spfs_parameter *sb, int dmn, char *block) {
    read_block(sb->device, 1 + dmn, block);
}
void set_directory_map_block(spfs_parameter *sb, int dmn, char *block) {
    write_block(sb->device, 1 + dmn, block);
}
void get_file_map(spfs_parameter *sb, int fmn, char *block) {
    int start = 1 + sb->directory_map_block_count + fmn;
    read_block(sb->device, start, block);
}
void set_file_map(spfs_parameter *sb, int fmn, char *block) {
    int start = 1 + sb->directory_map_block_count + fmn;
    write_block(sb->device, start, block);
}
/*void get_directory(int device, int dn, char *block) {
    spfs_parameter sb;
    get_system_block(device, &sb);
    int start = get_system_block_number(device) + 
        sb.directory_map_block_count + 
        sb.file_map_block_count + dn;
    read_block(device, start, block);
}*/
static void get_directory_block(spfs_parameter *sb, int dn, char *block) {
	int start = 1 + sb->directory_map_block_count + 
        		sb->file_map_block_count + dn;
    read_block(sb->device, start, block);
}
/*void set_directory(int device, int dn, char *block) {
    spfs_parameter sb;
    get_system_block(device, &sb);
    int start = get_system_block_number(device) + 
        sb.directory_map_block_count + 
        sb.file_map_block_count + dn;
    write_block(device, start, block);
}*/
static void set_directory_block(spfs_parameter *sb, int dn, char *block) {
	int start = 1 + sb->directory_map_block_count + 
        		sb->file_map_block_count + dn;
    write_block(sb->device, start, block);
}

void spfs_get_directory(spfs_parameter *sb, int dn, spfs_directory *d) {
    --dn; // number -> index
    int dir_blk_num = (dn/(sb->block_size/DIRECTORY_SIZE))+1;
    int dir_idx = dn%(sb->block_size/DIRECTORY_SIZE);
    get_directory_block(sb, dir_blk_num, block_buffer);
    spfs_directory *dirs = (spfs_directory *)block_buffer;
    memcpy(d, (dirs+dir_idx), DIRECTORY_SIZE);
}
void spfs_set_directory(spfs_parameter *sb, int dn, spfs_directory *d) {
    --dn; // number -> index
    int dir_blk_num = (dn/(sb->block_size/DIRECTORY_SIZE))+1;
    int dir_idx = dn%(sb->block_size/DIRECTORY_SIZE);
    get_directory_block(sb, dir_blk_num, block_buffer);
    spfs_directory *dirs = (spfs_directory *)block_buffer;
    memcpy(&dirs[dir_idx], d, DIRECTORY_SIZE);
    set_directory_block(sb, dir_blk_num, block_buffer);
}
void spfs_get_file(spfs_parameter *sb, int fn, spfs_file *f) {
    int start = 1 + sb->directory_map_block_count + 
        		sb->file_map_block_count + 
        		sb->directory_block_count + fn;
    read_block(sb->device, start, (char *)f);
}
void spfs_set_file(spfs_parameter *sb, int fn, spfs_file *f) {
    int start = 1 + sb->directory_map_block_count + 
        		sb->file_map_block_count + 
        		sb->directory_block_count + fn;
    write_block(sb->device, start, (char *)f);
}

// file organization module
/*
    获取 file_count 个可用的 file，并将他们串联，
    然后返回它们头节点的编号。
    return 的编号从 1 编号到 file_block_count。
 */
int get_free_files(spfs_parameter *sb, int file_count) {
    // 获取 system block，判断剩余的 file block 是否满足 file_count
    if (sb->free_file_count < file_count)
        return 0;
    int block_size = sb->block_size;
    char *file_map = block_buffer;
    sb->free_file_count -= file_count;
    spfs_file *file = &file_buffer;
    memset(file, 0, block_size);
    int bit_count = block_size * 8;
    // 串联选中的 free file block
    int file_head = 0;
    for (int j = 0; j < sb->file_map_block_count && file_count; ++j) {
        get_file_map(sb, j+1, file_map);
        int last_file_number = 0;
        int is_file_map_updated = 0;
        for (int i = 0; i < bit_count && file_count; ++i) {
            int curr_file_number = j * bit_count + i + 1;
            if (!test_bit(file_map, bit_count, i)) {
                set_bit(file_map, bit_count, i);
                is_file_map_updated = 1;
                // 记录头节点
                if (file_head == 0) { 
                    file_head = curr_file_number;
                }
                // 串联其他节点
                if (last_file_number != 0) {
                    file->next_file = curr_file_number;
                    spfs_set_file(sb, last_file_number, file); // 将上一个 file 与当前 file 串联
                }
                last_file_number = curr_file_number;
                //get_file(device, last_file_number, file);
                --file_count;
            }
        }
        if (is_file_map_updated) {
            set_file_map(sb, j+1, file_map);
        }
    }
    set_system_block(sb);
    return file_head;
}

int get_free_directory(spfs_parameter *sb) { 
    // 获取 system block，判断是否有剩余的 directory item
    if (sb->free_directory_count <= 0)
        return 0;
    //char directory[BLOCK_SIZE];
    int block_size = sb->block_size;
    char *directory_map = block_buffer;
    int directory_number = 0;
    int is_directory_map_updated = 0;
    int bit_count = block_size * 8;
    // 遍历 directory map 寻找可用的 directory item 的编号
    for (int j = 0; j < sb->directory_map_block_count && !directory_number; ++j) {
        get_directory_map_block(sb, j + 1, directory_map);
        for (int i = 0; i < bit_count && !directory_number; ++i) {
            int curr_directory_number = j * bit_count + i + 1;
            if (!test_bit(directory_map, bit_count, i)) {
                set_bit(directory_map, bit_count, i);
                is_directory_map_updated = 1;
                directory_number = curr_directory_number;
                --sb->free_directory_count;
            }
        }
        // 如果已找到一个 directory item 则返回
        if (is_directory_map_updated) {
            set_directory_map_block(sb, j + 1, directory_map);
            break;
        }
    }
    set_system_block(sb);
    return directory_number;
}

int get_directory_by_filename(spfs_parameter *sb, char *fname, spfs_directory *dir) {
    char *directory = block_buffer;//[BLOCK_SIZE];
    int block_size = sb->block_size;
    char *directory_map = block_buffer;//[BLOCK_SIZE];
    // 遍历 directory map 找到有效的 directory
    int bit_count = block_size * 8;
    for (int j = 0; j < sb->directory_map_block_count; ++j) {
        get_directory_map_block(sb, j + 1, directory_map);
        for (int i = 0; i < bit_count; ++i) {
            int directory_number = j * bit_count + i + 1;
            if (test_bit(directory_map, bit_count, i)) {
                --directory_number;
                int dn1 = directory_number / DIRECTORY_SIZE;
                int dn2 = directory_number % DIRECTORY_SIZE;
                get_directory_block(sb, dn1 + 1, directory);
                spfs_directory *ds = (spfs_directory *)directory;
                if (ds[dn2].file_head) {
                    if (!strncmp(fname, ds[dn2].name, FILE_NAME_LEN)) {
                        spfs_get_directory(sb, directory_number+1, dir);
                        return directory_number+1;
                    }
                }
                get_directory_map_block(sb, j + 1, directory_map);
            }
        }
    }
    return 0;
}

int get_file_by_filename(spfs_parameter *sb, char *fname, spfs_file *file) {
    spfs_directory dir;
    int ret;
    if ((ret = get_directory_by_filename(sb, fname, &dir)) != 0) {
        spfs_get_file(sb, dir.file_head, file);
    }
    return ret;
}

