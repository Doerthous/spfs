#ifndef _SPFS_DEVICES_H_
#define _SPFS_DEVICES_H_

#include <stdio.h>

#define SECTOR_SIZE 512
#define DEVICE_COUNT 1
#define HD0 0

int init_device(char *filename);
void close_device();
int read_sector(int device, int sector_number, char buf[]);
int write_sector(int device, int sector_number, char buf[]);

#endif // _SPFS_DEVICES_H_
