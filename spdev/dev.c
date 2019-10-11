#include "dev.h"

#define DEVICE_COUNT 1

typedef struct {
	char signature[16];
	int sector_size;
	int sector_count;
	int :32;
	int :32;
} device_rom;

typedef struct {
	FILE *device;
	int device_number;
	int sector_size;
	int sector_count;
} device_t;

static device_t devices[DEVICE_COUNT];

// sector_number  > 0, 0 sector for device rom
static inline void to_sector(int device, int sector_number) {
	if (sector_number < 1) {
		sector_number = 1;
	}
	else if (sector_number >= devices[device].sector_count) {
		sector_number = devices[device].sector_count;
	}
	fseek(devices[device].device, sector_number*devices[device].sector_size, SEEK_SET);
}
void create_device(char *filename, int sector_size, int sector_count) {
	// 设备信息
	device_rom rom;
	memset((void *)&rom, 0, sizeof(device_rom));
	memcpy(rom.signature, "DTS1", 4);
	rom.sector_size = sector_size;
	rom.sector_count = sector_count;

	// 创建设备
	FILE *img = fopen(filename, "wb");
	char buf[MAX_SECTOR_SIZE]; 
	memset((void *)buf, 0, MAX_SECTOR_SIZE);
	memcpy(buf, &rom, sizeof(device_rom));
	fwrite(buf, rom.sector_size, 1, img);
	memset((void *)buf, 0, MAX_SECTOR_SIZE);
	for (int i = 1; i < sector_count; ++i) {
		fwrite(buf, rom.sector_size, 1, img);
	}
	
	fclose(img);
}
int open_device(char *filename) {
	// 寻找可用的缓存
	// 若都不可用或设备不存在则返回 DEVICE_OPEN_FAILED，
	// 否则返回代表设备的编号
	int free_device_number = 0;
	for (; free_device_number < DEVICE_COUNT; ++free_device_number) {
		if (!devices[free_device_number].device) {
			break;
		}
	}
	if (free_device_number != DEVICE_COUNT) {
		FILE *d = fopen(filename, "rb+");
		if (d) {
			device_rom rom;
			fread(&rom, sizeof(device_rom), 1, d);
			if(!strncmp(rom.signature, "DTS1", 4)) {
				devices[free_device_number].device = d;
				devices[free_device_number].device_number = free_device_number;
				devices[free_device_number].sector_size = rom.sector_size;
				devices[free_device_number].sector_count = rom.sector_count;
				return free_device_number;
			}
		}
	}
	return DEVICE_OPEN_FAILED;
}
void close_device(int device) {
	if (0 <= device && device < DEVICE_COUNT) {
		if (devices[device].device) {
			fclose(devices[device].device);
			devices[device].device = NULL;
			devices[device].device_number = DEVICE_OPEN_FAILED;
			devices[device].sector_size = 0;
		}
	}
}
static int rw_sector(int d, int sn, void *b, int bs, size_t (*rw)(void *, size_t, size_t, FILE *)) {
	to_sector(d, sn);
	int sector_size = devices[d].sector_size;
	int rw_cnt = 0; 
	if (bs < sector_size) {
		rw_cnt = rw(b, bs, 1, devices[d].device);
	} else {
		rw_cnt = rw(b, sector_size, 1, devices[d].device);
	}
	return rw_cnt;
}
int read_sector(int device, int sector_number, char buf[], int buf_size) {
	return rw_sector(device, sector_number, buf, buf_size, fread);
}
int write_sector(int device, int sector_number, char buf[], int buf_size) {
	return rw_sector(device, sector_number, buf, buf_size, 
		(size_t (*)(void *, size_t, size_t, FILE *))fwrite);
}
int get_sector_size(int device) {
	return devices[device].sector_size;
}
int get_sector_count(int device) {
	return devices[device].sector_count;
}
