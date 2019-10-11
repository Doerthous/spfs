#include <stdio.h>
#include "spsh/spsh.h"
#include "spdev/vdev.h"

int upint(int a, int b) { // a/b
	int c = a/b;
	int d = a%b;
	if (d) {
		++c;
	}
	return c;
}
void mkspfs(int device, int boot) {
	// 获取设备容量及扇区大小
	int sector_size = get_sector_size(device);
	int sector_count = get_sector_count(device);

	// 每 block 的 sector 数
	// block 大小与数量
	int sector_count_per_block = 1;
	if (boot) {
		--sector_count; // 预留一个扇区做引导
	} 
	int capacity = sector_count*sector_size;
	int max_dir_count = (capacity/(256*1024))*32; // 256k 对应 32 个文件名
	int block_count = sector_count/sector_count_per_block;
	int block_size = sector_count_per_block * sector_size;

	// 根据给定目录名的数量估计文件系统参数
	int dmbc = upint(max_dir_count, 8*block_size);
	int dbc = upint(32*max_dir_count, block_size);
	int fmbc = upint(block_count-dmbc-dbc-1, 8*block_size);
	int fbc = block_count - dmbc - dbc - fmbc - 1;
	//printf("%d,%d,%d,%d\n", dmbc, dbc, fmbc, fbc);
	if (fmbc <= 0 || fbc <= 0) { // failed
		return;
	}

	// 设置 system block 信息
	spfs_parameter sb;
	sb.signature[0] = 's';
	sb.signature[1] = 'p';
	sb.signature[2] = 'f';
	sb.signature[3] = 's';
	sb.has_boot_sector = 0;
	sb.directory_count = dbc*block_size/32;				// 1024*32
	sb.file_count = fbc;								// 1024*64
	sb.free_directory_count = dbc*block_size/32;
	sb.free_file_count = fbc;
	sb.directory_map_block_count = dmbc;				//
	sb.file_map_block_count = fmbc;						//
	sb.directory_block_count = dbc;						//
	sb.file_block_count = fbc;							//
	sb.block_size = block_size;
	sb.sector_count_per_block = sector_count_per_block;
	sb.device = device;

	// 写入 device
	if (boot) {
		sb.has_boot_sector = 1;
		write_sector(device, 2, (char *)&sb, sizeof(sb));
	} else {
		write_sector(device, 1, (char *)&sb, sizeof(sb));
	}

	// 创建根目录
	spfs_directory directory;
	int directory_number = get_free_directory(&sb);
	if (directory_number != 1) {
		printf("Make fs failed, this device is not empty.\n");
		return;
	}
	memset(&directory, 0, sizeof(spfs_directory));
	memcpy(directory.name, "/", 1);
	directory.type = 1;
	spfs_set_directory(&sb, directory_number, &directory);

	set_system_block(&sb);
}

void print_usage() {
	printf("Usage: \n");
	printf("    spfs -c type imagename\n");
    printf("        create a img file\n");
	printf("    spfs -m [b] imagename\n");
    printf("        make file system on imagename, b: set first sector as boot sector\n");
    printf("    spfs -i imagename filename [newname]\n");
    printf("        insert file to imagename\n");
    printf("    spfs -s imagename\n");
    printf("        shell\n\n");
    printf("    type: DTS1, W25Q16, W25Q64\n");
}

int main(int argc, char *args[]) {
	if (argc < 2) {
		print_usage();
	}
	else if (!strcmp(args[1], "-c")) { // spfs -c type imagename
		if (argc != 4) {
			print_usage();
		}
		else {
			if (!strncmp(args[2], "DTS1", 4)) {
				create_vdevice(args[3], 512, 32*1024);
				printf("%s has created\n", args[3]);
			} 
			else if (!strncmp(args[2], "W25Q16", 6)) {
				create_vdevice(args[3], 4096, 512);
				printf("%s has created\n", args[3]);
			}
			else if (!strncmp(args[2], "W25Q64", 6)) {
				create_vdevice(args[3], 4096, 2048);
				printf("%s has created\n", args[3]);
			}
			else {
				print_usage();
			}
		}
	}
	else if (!strcmp(args[1], "-m")) { // spfs -m [b] imagename
		int b = 0;
		char *img;
		if (argc == 3) {
			img = args[2];
		}
		else if (argc == 4 && !strcmp(args[2], "b")) {
			img = args[3];
			b = 1;
		}
		else {
			print_usage();
			return 0;
		}
		int d = open_vdevice(img);
		if (d != DEVICE_OPEN_FAILED) {
			mkspfs(d, b);
			close_vdevice(d);
		}
		else {
			//
		}
	}
	else if (!strcmp(args[1], "-i")) { // spfs -i imagename filename [newname]
		if (argc > 3) {
			int d = open_vdevice(args[2]);
			if (d != DEVICE_OPEN_FAILED) {
				spfs_parameter sb;
				shell_env *sh_env = get_shell_env();
				get_system_block(d, &sh_env->fs_sys_blk);
				sh_env->ch2root();
				ist(argc-2, args+2);
				set_system_block(&sh_env->fs_sys_blk);
				close_vdevice(d);
			}
			else {
				printf("Open device %s failed!\n", args[2]);
			}
		}
		else {
			print_usage();
		}
	}
	else if (!strcmp(args[1], "-s")) { // spfs -d imagename
		if (argc == 3) {
			int d = open_vdevice(args[2]);
			if (d != DEVICE_OPEN_FAILED) {
				shell(d);
				close_vdevice(d);
			}
			else {
				printf("Open device %s failed!\n", args[2]);
			}
		}
		else {
			print_usage();
		}
	}
	else {
		print_usage();
	}
}
