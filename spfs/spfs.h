#ifndef _SPFS_H_
#define _SPFS_H_

#include <stdio.h>
#include <string.h>
#include "../spdev/dev.h"

// block number >= 1
//#define BLOCK_SIZE DEVICE_SECTOR_SIZE
#define MAX_BOOT_BLOCK_COUNT  10
#define MAX_BLOCK_SIZE        4096
//#define DIRECTORY_COUNT_PER_BLOCK   (BLOCK_SIZE/DIRECTORY_SIZE)
/*
    The first dev (block size: 512 B)

    block number    block name       block count
                _____________________
        1      |                     |
               |     boot block      | 1 block
               |_____________________|
        2      |                     |
               |    system block     | 1 block
               |_____________________|
        3      |                     | 
               | directory map block | 8 block
        10     |_____________________|
        11     |                     |
               |   file map block    | 16 block
        26     |_____________________|
        27     |                     |
               |   directory block   | 2048 block 
        2074   |_____________________|
        2075   |                     |
               |     file block      | 65536 block
        67610  |_____________________|

    67610 block = 67610 * 512 B = 33805 KB = 33 MB
*/


/*
    The W25Q16 flash (block size: 4096 B)

    block number    block name       block count
                _____________________
        1      |                     |
               |    system block     | 1 block
               |_____________________|
        2      |                     | 
               | directory map block | 1 block
               |_____________________|
        3      |                     |
               |   file map block    | 1 block
               |_____________________|
        4      |                     |
               |   directory block   | 1 block 
               |_____________________|
        5      |                     |
               |     file block      | 508 block
        512    |_____________________|

   512 block = 512 * 4096 B = 2048 KB = 2 MB

*/

struct spfs_b { // 1block

};
typedef struct {          
    // file system configuration infomation
    char    signature[4];               // "spfs"
    int     has_boot_sector;           /*< the number of block used by boot, must less than MAX_BOOT_BLOCK_COUNT */
    //      system_block_count = 1
    int     directory_map_block_count;  /*< */
    int     file_map_block_count;       /*< 一个 file block 对应一个 file。*/
    int     directory_block_count;      //
    int     file_block_count;           //
    // file system infomation
    int     directory_count;            /*< the number of directory item */ 
    int     file_count;                 /*< the number of file item */ 
    int     free_directory_count;       
    int     free_file_count;
    //
    int     block_size;
    int     sector_count_per_block;     // 每个 block 所对应的 sector 数量
    //
    int 	device;
} spfs_parameter;
typedef struct { // the format of directory item
#define FILE_NAME_LEN 14 // last character for 0
    int     file_head; // 如果是目录，则指向父目录，如果是文件，则指向文件数据块的头节点。
    int     next_directory;
    int     child_dir; // 子节点
    int     size;
    char    type; // 0 for file, 1 for directory
    char    name[FILE_NAME_LEN+1];
} spfs_directory;
typedef struct { // the format of file item, its size is BLOCK_SIZE 
    int     next_file;              
    char    data[MAX_BLOCK_SIZE-sizeof(int)];
} spfs_file;




/*
    low level api
 */
/*
    dev > 0
    获取 system block。
 */
void get_system_block(int dev, spfs_parameter *sb);
void set_system_block(spfs_parameter *sb);
/*
    获取 directory map block。
    dmbn：directory map 编号，取值范围为 [1, spfs_parameter.directory_map_block_count]。
 */
void get_directory_map_block(spfs_parameter *sb, int dmbn, char *blk);
/*
    dbn；directory block 编号，取值范围为 [1, spfs_parameter.directory_block_count]
 */
/*void get_directory(int dev, int dbn, char *blk);
void set_directory(int dev, int dbn, char *blk);*/




/*
    获取 cnt 个可用的 file，并将他们串联，然后返回它们头节点的编号。
    file 编号的取值范围为 [1, spfs_parameter.file_count]。
 */
int get_free_files(spfs_parameter *sb, int cnt);
/*
    获取一个可用的 directory 并返回其编号。
    directory 编号的取值范围为 [1, spfs_parameter.directory_count]。
 */
int get_free_directory(spfs_parameter *sb);




/*
    high level api
 */
/*
    dn；directory 编号，取值范围为 [1, spfs_parameter.directory_count]
 */
void spfs_get_directory(spfs_parameter *sb, int dn, spfs_directory *d);
void spfs_set_directory(spfs_parameter *sb, int dn, spfs_directory *d);
/*
    fn：file 编号，取值范围为 [1, spfs_parameter.file_count]
 */
void spfs_get_file(spfs_parameter *sb, int fn, spfs_file *f);
void spfs_set_file(spfs_parameter *sb, int fn, spfs_file *f);
/*
    通过 file 名字获取其对应的 directory 并返回其编号。
    directory 编号的取值范围为 [1, spfs_parameter.directory_count]。
    如果 file 不存在则返回 0。
 */
int get_directory_by_filename(spfs_parameter *sb, char *fname, spfs_directory *dir);
/*
    通过 file 名字获取其对应的 file 头节点并返回其编号。
    file 编号的取值范围为 [1, spfs_parameter.file_count]。
    如果file 不存在则返回 0。
 */
int get_file_by_filename(spfs_parameter *sb, char *fname, spfs_file *f);

#endif // _SPFS_H_
