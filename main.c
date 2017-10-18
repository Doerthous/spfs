#include <stdio.h>
#include <stdlib.h>
#include "spfs_devices.h"
#include "spfs_shell.h"

void create(char * filename) {
	struct spfs_system_block fs;
	fs.directory_count = 32768;				// 1024*32
	fs.file_count = 65536;					// 1024*64
	fs.free_directory_count = 32768;
	fs.free_file_count = 65536;
	fs.directory_map_block_count = 8;		//
	fs.file_map_block_count = 16;			//
	fs.directory_block_count = 2048;			//
	fs.file_block_count = 65536;				//

	FILE *img = fopen(filename, "wb");

	char buf[BLOCK_SIZE]; // initiate with 0
	memset(buf, 0, BLOCK_SIZE);
	// boot
	fwrite(buf, 1, BLOCK_SIZE, img);

	memcpy(buf, &fs, sizeof(struct spfs_system_block));
	// file system 
	fwrite(buf, 1, BLOCK_SIZE, img);

	memset(buf, 0, BLOCK_SIZE);
	// directory map, file map, directory, file
	for (int i = 0; i < 67608; ++i) 
		fwrite(buf, 1, BLOCK_SIZE, img);

	fclose(img);
}
void print_usage() {
	printf("Usage: \n");
	printf("    spfs -c imagename\n");
	printf("    spfs -s imagename\n");
	printf("    spfs -i imagename filename [as name]");
}
int main(int argc, char *args[]) {
	if (argc < 2) {
		print_usage();
	}
	else if (!strcmp(args[1], "-c")) {
		if (argc != 3) {
			print_usage();
		}
		else {
			create(args[2]);
			printf("%s has created\n", args[2]);
		}
	}
	else if (!strcmp(args[1], "-s")) {
		if (argc != 3) {
			print_usage();
		}
		else if (!init_device(args[2])) {
			printf("Device %s init failed\n", args[2]);
		}
		else {
			shell();
		}
	}
	else if (!strcmp(args[1], "-i")) {
		if (argc != 4 && argc != 5) {
			print_usage();
		}
		else if (!init_device(args[2])) {
			printf("Device %s init failed\n", args[2]);
		}
		else if (argc == 4 && insert(HD0, args[3])) {
			printf("Insert %s into %s successed\n", args[3], args[2]);
		}
		else if (argc == 5 && insert_as(HD0, args[3], args[4])) {
			printf("Insert %s into %s successed\n", args[3], args[2]);
		}
		else {
			printf("Insert %s into %s failed\n", args[3], args[2]);
		}
	}
	else if (!strcmp(args[1], "-d")) {
		if (argc != 3) {
			print_usage();
		}
		else if (!init_device(args[2])) {
			printf("Device %s init failed\n", args[2]);
		}
		else {
			disk(HD0);
		}
	}
	close_device();
	return 0;
}
