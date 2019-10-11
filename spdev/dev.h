#ifndef _SPFS_DEVICES_H_
#define _SPFS_DEVICES_H_

#include <stdio.h>
#include <string.h>

#define DEVICE_OPEN_FAILED 	-1
#define MIN_SECTOR_SIZE 	512
#define MAX_SECTOR_SIZE		4096

// sector_size: 扇区大小，单位字节
void create_device(char *filename, int sector_size, int sector_count);
//
int open_device(char *filename);
void close_device(int device);
// sector_number >= 0
int read_sector(int device, int sector_number, char buf[], int buf_size);
int write_sector(int device, int sector_number, char buf[], int buf_size);
//
int get_sector_size(int device);
int get_sector_count(int device);

#endif // _SPFS_DEVICES_H_
