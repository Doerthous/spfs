#ifndef _SPFS_SHELL_H_
#define _SPFS_SHELL_H_

#include <math.h>
#include "spfs.h"

void shell();

// commands
int ls(int device);
void cat(int device, char *filename);
void disk(int device);
void move(int device, char *filename, char *newname);
int insert_as(int device, char *file, char *filename);
int insert(int device, char *file);

#endif // _SPFS_SHELL_H_