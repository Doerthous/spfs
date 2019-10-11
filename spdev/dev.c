#include "dev.h"

#ifndef NULL
#define NULL 0
#endif

#define DEVICE_COUNT 1

static device_t *devices[DEVICE_COUNT];

int add_device(device_t *device) {
	// 寻找可用的缓存
	// 若都不可用或设备不存在则返回 DEVICE_OPEN_FAILED，
	// 否则返回代表设备的编号
	int free_device_number = 0;
	for (; free_device_number < DEVICE_COUNT; ++free_device_number) {
		if (!devices[free_device_number]) {
			devices[free_device_number] = device;
			break;
		}
	}
	if (free_device_number != DEVICE_COUNT) {
		return free_device_number;
	}
	return ADD_DEVICE_FAILED;
}
void remove_device(int device) {
	if (device < DEVICE_COUNT) {
		devices[device] = NULL;
	}
}
int read_sector(int device, int sector_number, char buf[], int buf_size) {
	if (devices[device]) {
		return devices[device]->read_sector(devices[device]->device_number, 
											sector_number, buf, buf_size);
	}
	return 0;
}
int write_sector(int device, int sector_number, char buf[], int buf_size) {
	if (devices[device]) {
		return devices[device]->write_sector(devices[device]->device_number, 
											sector_number, buf, buf_size);
	}
	return 0;
}
int get_sector_size(int device) {
	return devices[device]->sector_size;
}
int get_sector_count(int device) {
	return devices[device]->sector_count;
}
