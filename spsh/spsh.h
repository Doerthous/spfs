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

void shell(int device);
shell_env *get_shell_env();

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