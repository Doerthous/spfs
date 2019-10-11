#include "spsh.h"
#include "spio.h"

void cd(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();
	int tg_dn = existed(FS_TYPE_DIR, args[1]); 
	if (!tg_dn) {
		printf("Directory %s not existed\n", args[1]);
	}
	else {
		sh_env->ch2dir(tg_dn);
	}
}