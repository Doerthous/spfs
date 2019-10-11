#include "spsh.h"

static void set_to_dir(int target_dn);
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
}

static void print_usage() {
	printf("usable commands: \n");
	printf("    ist filename [new filename]\n");
	printf("    ls [path]\n");
	printf("    cat filename \n");
	printf("    mv filename filename \n");
	printf("    dsk \n");
	printf("    exit \n");
}
int command(int argc, char *args[]) {
	if (argc <= 0) {
		print_usage();
	}
	else {
		if (!strcmp(args[0], "ls")) {
			ls(argc, args);
		}
		else if (!strcmp(args[0], "ist")) {
			ist(argc, args);
		}
		else if (!strcmp(args[0], "cat")) {
			cat(argc, args);
		}
		else if (!strcmp(args[0], "exit")) {
			return 1;
		}
		else if (!strcmp(args[0], "dsk")) {
			dsk(argc, args);
		}
		else if (!strcmp(args[0], "mv")) {
			mv(argc, args);
		}
		else if (!strcmp(args[0], "mkdir")) {
			mkdir(argc, args);
		}
		else if (!strcmp(args[0], "cd")) {
			cd(argc, args);
		}
		else if (!strcmp(args[0], "pwd")) {
			pwd(argc, args);
		}
		else {
			printf("%s: command not found\n", args[0]);
			print_usage();
		}
	}
	return 0;
}
void shell(int device) {
	int argc;
	char *args[8];
	char buf[160];
	int exit = 0;
	// 若当前目录不存在，设置当前目录为根目录
	get_system_block(device, &sh_env.fs_sys_blk);
	set_to_root_dir();

	while (!exit) {
		printf("%s> ", sh_env.curr_dir.name);
		fgets(buf, 160, stdin);
		argc = 0;
		args[argc] = buf;
		for (int i = 0; buf[i]; ++i) {
			if (buf[i] == ' ') {
				buf[i] = 0;
				++argc;
				args[argc] = buf+i+1;
			}
			if (buf[i] == '\n') {
				buf[i] = 0;
				++argc;
				break;
			}
		}
		exit = command(argc, args);
	}

	set_system_block(&sh_env.fs_sys_blk);
}







