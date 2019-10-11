#include "spsh.h"
#include "spio.h"

void cd(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();
	int tg_dn; // target directory number
	if (!(tg_dn = existed(1, args[1]))) {
		printf("Directory %s not existed\n", args[1]);
	}
	else {
		sh_env->ch2dir(tg_dn);
	}
}