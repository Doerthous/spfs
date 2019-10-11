#ifndef _VDEV_H_
#define _VDEV_H_

#include "dev.h"

#define DEVICE_OPEN_FAILED 	-1

void create_vdevice(char *filename, int sector_size, int sector_count);
int open_vdevice(char *filename);
void close_vdevice(int device);

#endif // _VDEV_H_
