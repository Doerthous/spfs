#include "spsh.h"
#include "spio.h"

void ls(int argc, char *args[]) {
	shell_env *shenv = get_shell_env();
	// 遍历当前目录下的所有文件
	if (argc == 1) {
		spfs_directory dir;
		int next_directory = shenv->curr_dir.child_dir;
		while (next_directory) {
			spfs_get_directory(&shenv->fs_sys_blk, next_directory, &dir);
			if (dir.type) {
				printf("%-4c %-16s -\n", 'd', dir.name);
			} 
			else {
				printf("%-4c %-16s %dB\n", '-', dir.name, dir.size);
			}
			next_directory = dir.next_directory;
		}
	}
	else {
		int old_dn = shenv->curr_dir_num;
		int target_dn;
		if (!(target_dn = existed(1, args[1]))) {
			printf("Path %s not existed\n", args[1]);
		}
		else {
			shenv->ch2dir(target_dn);
			ls(1, NULL);
			shenv->ch2dir(old_dn);
		}
	}
}


