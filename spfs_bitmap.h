#ifndef _SPFS_BITMAP_H_
#define _SPFS_BITMAP_H_

void set_bit(void *bitmap, int length, int i);
void clear_bit(void *bitmap, int length, int i);
int test_bit(void *bitmap, int length, int i);

#endif // _SPFS_BITMAP_H_