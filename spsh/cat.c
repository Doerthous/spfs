#include "spsh.h"
#include "spio.h"

void cat(int argc, char *args[]) {
	shell_env *shenv = get_shell_env();

	// 遍历当前目录下的所有文件，寻找名为 filename 的文件
	spfs_directory dir;
	spfs_file file;

	int tg_dn; // target directory number
	if(!(tg_dn = existed(0, args[1]))) {
		printf("File %s not found\n", args[1]);
		return;
	}

	spfs_get_directory(&shenv->fs_sys_blk, tg_dn, &dir);
	file.next_file = dir.file_head;
	int block_size = shenv->fs_sys_blk.block_size;
	while (file.next_file) {
		spfs_get_file(&shenv->fs_sys_blk, file.next_file, &file);
		for (int i = 0; i < block_size - 4; ++i) {
			if (file.data[i]) {
				printf("%c", file.data[i]);
			}
		}
	};
	printf("\n");
}


