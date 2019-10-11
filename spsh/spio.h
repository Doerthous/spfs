#ifndef _SPIO_H_
#define _SPIO_H_

#include "spsh.h"

#define FS_TYPE_DIR  SPFS_TYPE_DIR
#define FS_TYPE_FILE SPFS_TYPE_FILE

#define FILE_OPEN_WRITE		0
#define FILE_OPEN_READ	 	1
#define FILE_OPEN_APPEND	2

typedef struct  {
	int file;
	int mode:4;
	int is_open:1;
} SP_FILE;
int existed(int type, char *filename);

int open(const char *filename, int mode);

int read(int file, int offset, char buf[], int size);

int write(int file, char buf[], int size);

void close(int file);

#endif // _SPIO_H_


