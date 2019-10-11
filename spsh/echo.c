#include "spsh.h"
#include "spio.h"

void echo(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();
	int i = 0, file = 0;
	if (argc > 1) {
		if (argc == 4 && !strcmp(args[argc-2], ">>")) {
			for (i = 1; i < argc - 2; ++i) {
				file = open(args[argc-1], FILE_OPEN_APPEND);
				write(file, args[1], strlen(args[1]));
				close(file);
			}
		}
		else if (argc == 2) {
			for (i = 1; i < argc; ++i) {
				printf("%s ", args[i]);
			}
			printf("\n");
		}
	}
}