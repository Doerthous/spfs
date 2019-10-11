#include "spfs.h"


static void set_bit(void *bitmap, int length, int i) {
    char *p = (char *)bitmap;
    const int byte = sizeof(char) * 8;
    if (i < length) {
        p[i / byte] |= (1 << (i % byte));
    }
}
static void clear_bit(void *bitmap, int length, int i) {
    char *p = (char *)bitmap;
    const int byte = sizeof(char) * 8;
    if (i < length) {
        p[i / byte] &= ~(1 << (i % byte));
    }
}
static int test_bit(void *bitmap, int length, int i) {
    char *p = (char *)bitmap;
    const int byte = sizeof(char) * 8;
    if (i < length) {
        int byte = sizeof(char) * 8;
        return p[i / byte] & (1 << (i % byte));
    }
    return 0;
}



static void read_block(int device, int bn, char *buf) {
	--bn;
	read_sector(device, bn, buf);
}
static void write_block(int device, int bn, char *buf) {
	--bn;
	write_sector(device, bn, buf);
}
void get_system_block(int device, struct spfs_system_block *fs) {
	char buf[BLOCK_SIZE];
	read_block(device, 2, buf);
	memcpy(fs, buf, sizeof(struct spfs_system_block));
}
void set_system_block(int device, struct spfs_system_block *fs) {
	char buf[BLOCK_SIZE];
	memset(buf, 0, BLOCK_SIZE);
	memcpy(buf, fs, sizeof(struct spfs_system_block));
	write_block(device, 2, buf);
}
void get_directory_map(int device, int dmn, char *block) {
	int start = (2 + dmn);
	read_block(device, start, block);
}
void set_directory_map(int device, int dmn, char *block) {
	int start = (2 + dmn);
	write_block(device, start, block);
}
void get_file_map(int device, int fmn, char *block) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	read_block(device, 2 + sb.directory_map_block_count + fmn, block);
}
void set_file_map(int device, int fmn, char *block) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	write_block(device, 2 + sb.directory_map_block_count + fmn, block);
}
void get_directory(int device, int dn, char *block) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	read_block(device, 2 + sb.directory_map_block_count 
						+ sb.file_map_block_count + dn, block);
}
void set_directory(int device, int dn, char *block) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	write_block(device, 2 + sb.directory_map_block_count 
						+ sb.file_map_block_count + dn, block);
}
void get_file(int device, int fn, char *block) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	read_block(device, 2 + sb.directory_map_block_count
		+ sb.file_map_block_count + sb.directory_block_count + fn, block);
}
void set_file(int device, int fn, char *block) {
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	write_block(device, 2 + sb.directory_map_block_count
		+ sb.file_map_block_count + sb.directory_block_count + fn, block);
}


// file organization module
int get_free_files(int device, int file_count) {
	char file_map[BLOCK_SIZE];
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	if (sb.free_file_count < file_count)
		return 0;
	sb.free_file_count -= file_count;
	char file[BLOCK_SIZE];
	memset(file, 0, BLOCK_SIZE);
	int file_head = -1;
	for (int j = 0; j < sb.file_map_block_count && file_count; ++j) {
		get_file_map(device, j+1, file_map);
		int last_file_number = -1;// j * 1024 + 1;
		int bit_count = BLOCK_SIZE * 8;
		for (int i = 0; i < bit_count && file_count; ++i) {
			int curr_file_number = j * BLOCK_SIZE + i + 1;
			if (!test_bit(file_map, bit_count, i)) {
				set_bit(file_map, bit_count, i);
				if (file_head == -1)
					file_head = curr_file_number;
				if (last_file_number != -1) {
					*((int *)file) = curr_file_number;
					set_file(device, last_file_number, file);
				}
				last_file_number = curr_file_number;
				get_file(device, last_file_number, file);
				--file_count;
			}
		}
		set_file_map(device, j+1, file_map);
	}
	set_system_block(device, &sb);
	return file_head;
}
int get_free_directory(int device) {
	char directory_map[BLOCK_SIZE];
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	if (sb.free_directory_count <= 0)
		return 0;
	char directory[BLOCK_SIZE];
	int directory_number = 0;
	for (int j = 0; j < sb.directory_map_block_count && !directory_number; ++j) {
		get_directory_map(device, j + 1, directory_map);
		int bit_count = BLOCK_SIZE * 8;
		for (int i = 0; i < BLOCK_SIZE && !directory_number; ++i) {
			int curr_directory_number = j * BLOCK_SIZE + i + 1;
			if (!test_bit(directory_map, bit_count, i)) {
				set_bit(directory_map, bit_count, i);
				directory_number = curr_directory_number;
				--sb.free_directory_count;
			}
		}
		set_directory_map(device, j + 1, directory_map);
	}
	set_system_block(device, &sb);
	return directory_number;
}


int name2directory(int device, char *filename) {
	char directory_map[BLOCK_SIZE];
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	char directory[BLOCK_SIZE];
	for (int j = 0; j < sb.directory_map_block_count; ++j) {
		get_directory_map(device, j + 1, directory_map);
		int bit_count = BLOCK_SIZE * 8;
		for (int i = 0; i < bit_count; ++i) {
			int directory_number = j * 1024 + i + 1;
			if (test_bit(directory_map, bit_count, i)) {
				--directory_number;
				const int sizeofdirectory = sizeof(struct spfs_directory);
				int dn1 = directory_number / sizeofdirectory;
				int dn2 = directory_number % sizeofdirectory;
				get_directory(device, dn1 + 1, directory);
				struct spfs_directory *ds = (struct spfs_directory *)directory;
				if (ds[dn2].file_head) {
					char __filename[16];
					memcpy(__filename, ds[dn2].name, 14);
					__filename[14] = 0;
					if (!strcmp(filename, __filename)) {
						return directory_number+1;
					}
				}
			}
		}
	}
	return 0;
}
int name2file(int device, char *filename) {
	char directory_map[BLOCK_SIZE];
	struct spfs_system_block sb;
	get_system_block(device, &sb);
	char directory[BLOCK_SIZE];
	for (int j = 0; j < sb.directory_map_block_count; ++j) {
		get_directory_map(device, j+1, directory_map);
		int bit_count = BLOCK_SIZE * 8;
		for (int i = 0; i < bit_count; ++i) {
			int directory_number = j * 1024 + i + 1;
			if (test_bit(directory_map, bit_count, i)) {
				--directory_number;
				const int sizeofdirectory = sizeof(struct spfs_directory);
				int dn1 = directory_number / sizeofdirectory;
				int dn2 = directory_number % sizeofdirectory;
				get_directory(device, dn1+1, directory);
				struct spfs_directory *ds = (struct spfs_directory *)directory;
				if (ds[dn2].file_head) {
					char __filename[16];
					memcpy(__filename, ds[dn2].name, 14);
					__filename[14] = 0;
					if (!strcmp(filename, __filename)) {
						return ds[dn2].file_head;
					}
				}
			}
		}
	}
	return 0;
}

