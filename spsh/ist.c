#include "spsh.h"
#include "spio.h"

int ist(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();

	char *org_nm = args[1]; // origin name
	char *new_nm = org_nm;
	if (argc == 3) {
		new_nm = args[2]; // new name
	}

	// 判断文件是否存在
	if(existed(0, new_nm)) {
		printf("File %s alreadys existed\n", org_nm);
		return 0;
	}

	// 打开文件
	FILE * f = fopen(org_nm, "rb");
	if (!f) {
		printf("Not such file: %s\n", org_nm);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	long size = ftell(f);

	// 计算存放文件所需的zone数量
	int block_size = sh_env->fs_sys_blk.block_size;
	int file_count = size / block_size;
	if (size%block_size > 0) {
		file_count += 1;
	}

	// 写入
	int file_head = get_free_files(&sh_env->fs_sys_blk, file_count);
	int zone_head = file_head;
	spfs_file file;
	fseek(f, 0, SEEK_SET);
	int insert_count = 0;
	while (zone_head) {
		spfs_get_file(&sh_env->fs_sys_blk, zone_head, &file);
		memset(file.data, 0, block_size-4);
		insert_count += fread(file.data, 1, block_size - 4, f);
		spfs_set_file(&sh_env->fs_sys_blk, zone_head, &file);
		zone_head = file.next_file;
	}
	// 关闭文件
	fclose(f);

	// 创建新的 directory
	spfs_directory directory;
	int name_len = strlen(new_nm);
	int directory_number = get_free_directory(&sh_env->fs_sys_blk);
	memset(&directory, 0, sizeof(spfs_directory));
	if (directory_number) {
		// 关联目录
		directory.next_directory = sh_env->curr_dir.child_dir;
		sh_env->curr_dir.child_dir = directory_number;
		// 设置文件信息
		directory.file_head = file_head;
		directory.size = size;
		memcpy(directory.name, new_nm, name_len < FILE_NAME_LEN ? name_len : FILE_NAME_LEN);
		spfs_set_directory(&sh_env->fs_sys_blk, directory_number, &directory);
		spfs_set_directory(&sh_env->fs_sys_blk, sh_env->curr_dir_num, &sh_env->curr_dir);
	}
	return insert_count;
}