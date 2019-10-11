#ifndef _SPFS_DEVICES_H_
#define _SPFS_DEVICES_H_

#include <stdio.h>
#include <string.h>

#define ADD_DEVICE_FAILED 	-1
#define MIN_SECTOR_SIZE 	512
#define MAX_SECTOR_SIZE		4096

typedef struct {
	int device_number;
	int sector_size;
	int sector_count;
	int (*read_sector)(int dev, int sec_num, char *buf, int buf_sz);
	int (*write_sector)(int dev, int sec_num, char *buf, int buf_sz);
} device_t;

// sector_number > 0
int read_sector(int device, int sector_number, char buf[], int buf_size);
int write_sector(int device, int sector_number, char buf[], int buf_size);
int get_sector_size(int device);
int get_sector_count(int device);
//
int add_device(device_t *device);
void remove_device(int device);

#endif // _SPFS_DEVICES_H_
