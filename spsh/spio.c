#include "spio.h"

static int __existed(int type, char *filename) {
	shell_env *shenv = get_shell_env();
	spfs_directory directory;
	if (!strcmp(filename, ".")) {
		return shenv->curr_dir_num; // current
	}
	if (!strcmp(filename, "..")) {
		if (shenv->curr_dir_num != 1) { //
			return shenv->curr_dir.file_head; // parent
		}
		else {
			return shenv->curr_dir_num; // root
		}
	}
	for (int i = 0; i <= strlen(filename); ++i) {
		if (!filename[i]) {
			return shenv->curr_dir_num; // current for ..., ...., ....., and so on.
		}
		if (filename[i] != '.') {
			break;
		}
	}
	int next_directory = shenv->curr_dir.child_dir;
	while (next_directory) {
		spfs_get_directory(&shenv->fs_sys_blk, next_directory, &directory);
		if (type == 2 || type == directory.type) {
			if (!strcmp(directory.name, filename)) {
				return next_directory;
			}
		}
		next_directory = directory.next_directory;
	}
	return 0;
}
int existed(int type, char *filename) {
	shell_env *shenv = get_shell_env();
	int old_dn = shenv->curr_dir_num; // 保存当前工作目录
	int target_dn = 0;
	char *p = filename, *q;
	if (p[0] == '/') {
		shenv->ch2root();
		++p;
		target_dn = 1;
	}
	while (*p) {
		for (q = p; *p && *p != '/'; ++p); 
		if (*p) {
			*p = 0;
			++p;
		} 
		if(!(target_dn = __existed(*p ? 1 : type, q))) { // *p == 0 意味着到达最后一个文件
			target_dn = 0;
			break;
		} else {
			shenv->ch2dir(target_dn);
		}
	}
	for (q = filename; q != p; ++q) { // 还原 filename
		if (!*q) {
			*q = '/';
		}
	}
	shenv->ch2dir(old_dn); // 恢复当前工作目录
	return target_dn;
}