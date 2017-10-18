#include "spfs_devices.h"

static FILE *devices[DEVICE_COUNT];

static inline void to_sector(int device, int sector_number) {
	fseek(devices[device], sector_number << 9, SEEK_SET);
}

int init_device(char *filename) {
	devices[0] = fopen(filename, "rb+");
	if (!devices[0])
		return 0;
	return 1;
}
void close_device() {
	for (int i = 0; i < DEVICE_COUNT; ++i) {
		if (devices[i]) {
			fclose(devices[i]);
			devices[i] = NULL;
		}
	}
}
int read_sector(int device, int sector_number, char buf[]) {
	to_sector(device, sector_number);
	return fread(buf, 1, SECTOR_SIZE, devices[device]);
}
int write_sector(int device, int sector_number, char buf[]) {
	to_sector(device, sector_number);
	return fwrite(buf, 1, SECTOR_SIZE, devices[device]);
}
