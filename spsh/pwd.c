#include "spsh.h"
#include "spio.h"

void pwd(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();

	char pwd[160];
	int size = 160;
	char *p = pwd;
	int dn = sh_env->curr_dir_num;
	spfs_directory *dir = &sh_env->curr_dir;
	do {
		if((size -= snprintf(p, size, "%s/", dir->name)) > 0) {
			spfs_get_directory(&sh_env->fs_sys_blk, dir->file_head, dir);
			while (*p++);
			--p;
		}
		else {
			printf("Buffer size %d is insufficient\n", 160);
		}
		
	} while (dir->file_head);
	for (--p; p >= pwd; --p) {
		printf("%c", *p);
	}
	printf("\n");
	spfs_get_directory(&sh_env->fs_sys_blk, sh_env->curr_dir_num, &sh_env->curr_dir);
}