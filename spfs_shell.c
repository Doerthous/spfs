#include "spfs_shell.h"

void shell() {
	char buf[80];
	char buf2[80];
	while (1) {
		printf("> ");
		scanf("%s", buf);
		if (!strcmp(buf, "ls")) {
			ls(HD0);
		}
		else if (!strcmp(buf, "ist")) {
			scanf("%s", buf);
			insert(HD0, buf);
		}
		else if (!strcmp(buf, "cat")) {
			scanf("%s", buf);
			cat(HD0, buf);
		}
		else if (!strcmp(buf, "exit")) {
			return;
		}
		else if (!strcmp(buf, "disk")) {
			disk(HD0);
		}
		else if (!strcmp(buf, "mv")) {
			scanf("%s", buf);
			scanf("%s", buf2);
			move(HD0, buf, buf2);
		}
		else {
			printf("%s: command not found\n", buf);
			printf("usable commands: \n", buf);
			printf("    ist filename \n", buf);
			printf("    ls \n", buf);
			printf("    cat filename \n", buf);
			printf("    mv filename filename \n", buf);
			printf("    disk \n", buf);
			printf("    exit \n", buf);
		}
	}
}

int ls(int device) {
	char directory_map[BLOCK_SIZE];
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	char directory[BLOCK_SIZE];
	for (int j = 0; j < sb.directory_map_block_count; ++j) {
		get_directory_map(device, j + 1, directory_map);
		int bit_count = BLOCK_SIZE * 8;
		for (int i = 0; i < bit_count; ++i) {
			int directory_number = j * 1024 + i + 1;
			if (test_bit(directory_map, bit_count, i)) {
				--directory_number;
				const int sizeofdirectory = sizeof(struct spfs_directory);
				int dn1 = directory_number / sizeofdirectory;
				int dn2 = directory_number % sizeofdirectory;
				get_directory(device, dn1+1, directory);
				struct spfs_directory *ds = (struct spfs_directory *)directory;
				if (ds[dn2].file_head) {
					char filename[16];
					memcpy(filename, ds[dn2].name, 14);
					filename[14] = 0;
					printf("%-24s %dByte\n", filename, ds[dn2].size);
				}
			}
		}
	}
}
void cat(int device, char *filename) {
	int file_head = name2file(device, filename);
	if (!file_head) {
		printf("File %s not found\n", filename);
		return;
	}
	char block[BLOCK_SIZE];
	while (file_head) {
		get_file(device, file_head, block);
		for (int i = 0; i < BLOCK_SIZE - 4; ++i) {
			printf("%c", block[4 + i]);
		}
		file_head = *((int *)block);
	}
	printf("\n");
}
void move(int device, char *filename, char *newname) {
	int directory_number = 0;
	if (!(directory_number = name2directory(device, filename))) {
		printf("Not such file: %s\n", filename);
		return;
	}
	char directory[BLOCK_SIZE];
	--directory_number;
	const int sizeofdirectory = sizeof(struct spfs_directory);
	int dn1 = directory_number / sizeofdirectory;
	int dn2 = directory_number % sizeofdirectory;
	get_directory(device, dn1 + 1, directory);
	struct spfs_directory *ds = (struct spfs_directory *)directory;
	if (ds[dn2].file_head) {
		memcpy(ds[dn2].name, newname, 14);
	}
	set_directory(device, dn1 + 1, directory);
}
void disk(int device) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	printf("Directorys:             %d\n", sb.directory_count);
	printf("Files:                  %d\n", sb.file_count);
	printf("Free directorys:        %d\n", sb.free_directory_count);
	printf("Free files:             %d\n", sb.free_file_count);
	printf("Directory map blocks:   %d\n", sb.directory_map_block_count);
	printf("File map blocks:        %d\n", sb.file_map_block_count);
	printf("Directory blocks:       %d\n", sb.directory_block_count);
	printf("File blocks:            %d\n", sb.file_block_count);
}
int insert_as(int device, char *file, char *filename) {
	if (!filename) {
		filename = file;
	}
	// 判断文件是否存在
	if (name2file(device, filename)) {
		printf("File %s already exists\n", filename);
		return 0;
	}

	// 打开文件
	FILE * f = fopen(file, "rb");
	if (!f) {
		printf("Not such file: %s\n", file);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	// 计算存放文件所需的zone数量

	int file_count = ceil(size / 508.0);
	int file_head = get_free_files(device, file_count);
	int zone_head = file_head;
	// 写入
	char block[BLOCK_SIZE];
	fseek(f, 0, SEEK_SET);
	int insert_count = 0;
	while (zone_head) {
		get_file(device, zone_head, block);
		insert_count += fread(block + 4, 1, BLOCK_SIZE - 4, f);
		set_file(device, zone_head, block);
		zone_head = *((int *)block);
	}
	// 关闭文件
	fclose(f);
	//
	int directory_number = get_free_directory(device);
	memset(block, 0, BLOCK_SIZE);
	if (directory_number) {
		--directory_number;
		const int sizeofdirectory = sizeof(struct spfs_directory);
		int dn1 = directory_number / sizeofdirectory;
		int dn2 = directory_number % sizeofdirectory;
		get_directory(device, dn1 + 1, block);
		struct spfs_directory *ds = (struct spfs_directory *)block;
		memcpy(ds[dn2].name, filename, strlen(filename) < 14 ? strlen(filename) : 14);
		ds[dn2].size = size;
		ds[dn2].file_head = file_head;
		set_directory(device, dn1 + 1, block);
	}
	return insert_count;
}
int insert(int device, char *file) {
	return insert_as(device, file, NULL);
}