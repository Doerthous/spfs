#include "vdev.h"

typedef struct {
	char signature[16];
	int sector_size;
	int sector_count;
	int :32;
	int :32;
} device_rom;

typedef struct {
	device_t dev;
	FILE *vdev;
} vdevice_t;


#define DEVICE_COUNT 1

static vdevice_t devices[DEVICE_COUNT];
// sector_number  > 0, 0 sector for device rom
static inline void to_sector(int device, int sector_number) {
	if (sector_number < 1) {
		sector_number = 1;
	}
	else if (sector_number >= devices[device].dev.sector_count) {
		sector_number = devices[device].dev.sector_count;
	}
	fseek(devices[device].vdev, sector_number*devices[device].dev.sector_size, SEEK_SET);
}

static int rw_sector(int d, int sn, void *b, int bs, size_t (*rw)(void *, size_t, size_t, FILE *)) {
	for (int i = 0; i < DEVICE_COUNT; ++i) {
		if (devices[i].dev.device_number == d) {
			d = i;
			to_sector(d, sn);
			int sector_size = devices[d].dev.sector_size;
			int rw_cnt = 0; 
			if (bs < sector_size) {
				rw_cnt = rw(b, bs, 1, devices[d].vdev);
			} else {
				rw_cnt = rw(b, sector_size, 1, devices[d].vdev);
			}
			return rw_cnt;
		}
	}
	return 0;
}
int vdevice_read_sector(int device, int sector_number, char buf[], int buf_size) {
	return rw_sector(device, sector_number, buf, buf_size, fread);
}
int vdevice_write_sector(int device, int sector_number, char buf[], int buf_size) {
	return rw_sector(device, sector_number, buf, buf_size, 
		(size_t (*)(void *, size_t, size_t, FILE *))fwrite);
}

void create_vdevice(char *filename, int sector_size, int sector_count) {
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
int open_vdevice(char *filename) {
	// 寻找可用的缓存
	// 若都不可用或设备不存在则返回 DEVICE_OPEN_FAILED，
	// 否则返回代表设备的编号
	int free_device_number = 0;
	for (; free_device_number < DEVICE_COUNT; ++free_device_number) {
		if (!devices[free_device_number].vdev) {
			break;
		}
	}
	if (free_device_number != DEVICE_COUNT) {
		FILE *d = fopen(filename, "rb+");
		if (d) {
			device_rom rom;
			fread(&rom, sizeof(device_rom), 1, d);
			if(!strncmp(rom.signature, "DTS1", 4)) {
				devices[free_device_number].vdev = d;
				devices[free_device_number].dev.device_number = free_device_number;
				devices[free_device_number].dev.sector_size = rom.sector_size;
				devices[free_device_number].dev.sector_count = rom.sector_count;
				devices[free_device_number].dev.read_sector = vdevice_read_sector;
				devices[free_device_number].dev.write_sector = vdevice_write_sector;
				int dev_num = add_device(&devices[free_device_number].dev);
				if (dev_num == ADD_DEVICE_FAILED) {
					return DEVICE_OPEN_FAILED;
				}
				devices[free_device_number].dev.device_number = dev_num;
				return dev_num;
			}
		}
	}
	return DEVICE_OPEN_FAILED;
}
void close_vdevice(int device) {
	if (0 <= device && device < DEVICE_COUNT) {
		if (devices[device].dev.device_number == device &&
			devices[device].vdev) {
			fclose(devices[device].vdev);
			devices[device].vdev = NULL;
			devices[device].dev.device_number = DEVICE_OPEN_FAILED;
			devices[device].dev.sector_size = 0;
			devices[device].dev.sector_count = 0;
			devices[device].dev.read_sector = NULL;
			devices[device].dev.write_sector = NULL;
			remove_device(devices[device].dev.device_number);
		}
	}
}

