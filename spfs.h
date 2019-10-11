#ifndef _SPFS_H_
#define _SPFS_H_

#include <stdio.h>
#include <string.h>
#include "spfs_devices.h"


// Block number: 1 to n, 0 for null
// 1block = 512B
#define BLOCK_SIZE 512

struct spfs_b { // 1block

};
struct spfs_system_block {			// 1block
	int directory_count;			// 1024*32
	int file_count;					// 1024*64
	int free_directory_count;		
	int free_file_count;
	int directory_map_block_count;	//
	int file_map_block_count;		//
	int directory_block_count;		//
	int file_block_count;			//
};
struct spfs_directory_map {			// 1024*32bit = 4KB = 8block

};
struct spfs_file_map {				// 1024*64bit = 8KB = 16block

};
struct spfs_directory {				// 1024*32*32B = 1MB = 2048block
#define FILE_NAME_LEN 24
	int file_head;
	int size;
	char name[FILE_NAME_LEN];
};
struct spfs_file {					// 1024*64*512B = 32MB = 65536block

};

	
void get_system_block(int device, struct spfs_system_block *fs);
void get_directory_map(int device, int dmn, char *block);

// file organization module
int get_free_files(int device, int file_count);
int get_free_directory(int device);

//
void get_directory(int device, int dn, char *block);
void set_directory(int device, int dn, char *block);
void get_file(int device, int fn, char *block);
void set_file(int device, int fn, char *block);

//
int name2directory(int device, char *filename);
int name2file(int device, char *filename);

#endif // _SPFS_H_