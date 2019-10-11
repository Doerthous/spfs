#include "spsh.h"
#include "spio.h"

void mkdir(int argc, char *args[]) {
	shell_env *shenv = get_shell_env();

	// 判断 path 是否存在
	spfs_directory dir;
	if(existed(2, args[1])) {
		printf("Path %s alreadys existed\n", args[1]);
	}

	int old_dn  = shenv->curr_dir_num;
	char *dirname = args[1], *end;
	if (dirname[0] == '/') {
		shenv->ch2root();
		++dirname;
	}

	int tg_dn; // target directory number
	for (end = dirname; *dirname; ++end) {
		if (*end == '/' || !*end) {
			if (*end == '/') {
				*end = 0;
				++end;
			}
			if (tg_dn = existed(1, dirname)) {
				shenv->ch2dir(tg_dn);
			} 
			else {
				// 创建目录
				int name_len = strlen(dirname);
				name_len = name_len < FILE_NAME_LEN ? name_len: FILE_NAME_LEN;
				tg_dn = get_free_directory(&shenv->fs_sys_blk);
				memset(&dir, 0, sizeof(spfs_directory));
				if (tg_dn) {
					// 关联
					dir.file_head = shenv->curr_dir_num; // 复用 file_head 关联父目录节点
					dir.next_directory = shenv->curr_dir.child_dir;
					shenv->curr_dir.child_dir = tg_dn;
					dir.type = 1;
					memcpy(dir.name, dirname, name_len);
					dir.name[name_len] = 0;
					spfs_set_directory(&shenv->fs_sys_blk, tg_dn, &dir);
					spfs_set_directory(&shenv->fs_sys_blk, shenv->curr_dir_num, &shenv->curr_dir);
				}
			}
			shenv->ch2dir(tg_dn);
			dirname = end;
		}
	}
	shenv->ch2dir(old_dn);
	//return tg_dn; // 成功则返回 directory 编号，否则返回 0。
}


