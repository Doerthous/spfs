#include <stdlib.h>
#include "spsh.h"
#include "spio.h"

int ist(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();

	char *org_nm = args[1]; // origin name
	char *new_nm = org_nm;
	if (argc == 3) {
		new_nm = args[2]; // new name
	}

	// 判断文件是否存在
	if(existed(FS_TYPE_FILE, new_nm)) {
		printf("File %s alreadys existed\n", org_nm);
		return 0;
	}

	// 打开文件
	FILE * f = fopen(org_nm, "rb");
	if (!f) {
		printf("Not such file: %s\n", org_nm);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *data = (char *)malloc(size);
	size = fread(data, 1, size, f);
	fclose(f);

	// 创建新的 directory
	int file = open(new_nm, FILE_OPEN_WRITE);
	size = write(file, data, size);
	close(file);

	free(data);
	return size;
}