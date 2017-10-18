#include "spfs_bitmap.h"

void set_bit(void *bitmap, int length, int i) {
	char *p = (char *)bitmap;
	const int byte = sizeof(char) * 8;
	if (i < length) {
		p[i / byte] |= (1 << (i % byte));
	}
}
void clear_bit(void *bitmap, int length, int i) {
	char *p = (char *)bitmap;
	const int byte = sizeof(char) * 8;
	if (i < length) {
		p[i / byte] &= ~(1 << (i % byte));
	}
}
int test_bit(void *bitmap, int length, int i) {
	char *p = (char *)bitmap;
	const int byte = sizeof(char) * 8;
	if (i < length) {
		int byte = sizeof(char) * 8;
		return p[i / byte] & (1 << (i % byte));
	}
	return 0;
}
