#ifndef _SPFS_SHELL_H_
#define _SPFS_SHELL_H_

#include "../spfs/spfs.h"

typedef struct {
	spfs_parameter fs_sys_blk;
	spfs_directory curr_dir;
	int curr_dir_num;

	void (*ch2dir)(int);
	void (*ch2root)();
} shell_env;


shell_env *get_shell_env();

void shell(int device);

/*static void set_to_dir(int target_dn);
static void set_to_root_dir();

shell_env sh_env = {
	.ch2dir = set_to_dir,
	.ch2root = set_to_root_dir,
};

static void set_to_dir(int target_dn) {
	spfs_get_directory(&sh_env.fs_sys_blk, target_dn, &sh_env.curr_dir);
	sh_env.curr_dir_num = target_dn;
}
static void set_to_root_dir() {
	set_to_dir(1);
}


shell_env *get_shell_env() {
	return &sh_env;
}*/

// commands
void ls(int argc, char *args[]);
void cat(int argc, char *args[]);
void mv(int argc, char *args[]);
void mkdir(int argc, char *args[]);
//void mkfs(int device);
void cd(int argc, char *args[]);
void pwd(int argc, char *args[]);
void dsk(int argc, char *args[]);
int ist(int argc, char *args[]);
#endif // _SPFS_SHELL_H_