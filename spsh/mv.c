#include "spsh.h"
#include "spio.h"

void mv(int argc, char *args[]) {
	shell_env *shenv = get_shell_env();

	// 当前目录问题
	int tg_dn = 0; // target directory number
	spfs_directory dir;
	if(!(tg_dn = existed(2, args[1]))) {
		printf("File %s not found\n", args[1]);
		return;
	}
	if(existed(2, args[2])) {
		printf("File %s alreadys existed\n", args[2]);
		return;
	}
	spfs_get_directory(&shenv->fs_sys_blk, tg_dn, &dir);
	memcpy(dir.name, args[2], FILE_NAME_LEN);
	spfs_set_directory(&shenv->fs_sys_blk, tg_dn, &dir);
}