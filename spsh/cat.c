#include "spsh.h"
#include "spio.h"


static void print(char data[], int size) {
	while (size--) {
		printf("%c", (*data++));
	}
}
void cat(int argc, char *args[]) {
	char data[256];
	int rsz = 0, rc = 0;
	int file = open(args[1], FILE_OPEN_READ);
	if(!file) {
		printf("File %s not found\n", args[1]);
		return;
	}
	rsz = 256;
	while (rsz == 256) {
		rsz = 256;
		rsz = read(file, rc, data, rsz);
		rc += rsz;
		print(data, rsz);
	}
	close(file);
	printf("\n");
}