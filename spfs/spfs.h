#ifndef _SPFS_H_
#define _SPFS_H_

/*
    spfs
    本系统在存储设备中的结构图如下：

    block number    block name       block count
                _____________________
        1      |                     |
               |    system block     | 1   blocks
               |_____________________|
        2      |                     | 
               | directory map block | a-2 blocks
               |_____________________|
        a      |                     |
               |   file map block    | b-a blocks
               |_____________________|
        b      |                     |
               |   directory block   | c-b blocks
               |_____________________|
        c      |                     |
               |     file block      | c-d blocks
        d-1    |_____________________|

        注：block 编号从 1 开始计数。

    以下是本文件系统的相关参数
*/
#define MAX_BLOCK_SIZE      4096
#define MAX_FILE_DATA_SIZE  (MAX_BLOCK_SIZE - sizeof(int))

/*
  spfs
    本文件系统使用了如下的宏定义函数
        SPFS_MEMSET
        SPFS_MEMCPY
        SPFS_STRNCMP
        SPFS_SET_BIT
        SPFS_CLEAR_BIT
        SPFS_TEST_BIT
    若上述函数无定义则使用本项目中默认的函数，具体实现在 spfs.c 中。
*/
//#define SPFS_MEMSET(mem, val, size)
//#define SPFS_MEMCPY(dest, src, size)
//#define SPFS_STRNCMP(str1, str2, n)
//#define SPFS_SET_BIT(bm, len, i)
//#define SPFS_CLEAR_BIT(bm, len, i)
//#define SPFS_TEST_BIT(bm, len, i)

/*
    此处将硬盘抽象，请定义如下宏函数
        int SPFS_READ_SECTOR(int device, int sector_number, int buffer, int size)
        int SPFS_WRITE_SECTOR(int device, int sector_number, int buffer, int size) 
*/
#include "../spdev/dev.h"
#define SPFS_READ_SECTOR(dev, sn, buf, size) \
            read_sector((dev), (sn), (buf), (size)) 
#define SPFS_WRITE_SECTOR(dev, sn, buf, size) \
            write_sector((dev), (sn), (buf), (size))
#define SPFS_GET_DEVICE_SECTOR_COUNT(dev) \
            get_sector_count((dev))
#define SPFS_GET_DEVICE_SECTOR_SIZE(dev) \
            get_sector_size((dev))

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
    int     file_head; // 如果是目录，则指向父目录，如果是文件，则指向文件数据块的头节点。
    int     next_directory;
    int     parent_dir;
    int     child_dir; // 子节点
    int     size;

    #define SPFS_TYPE_DIR  1
    #define SPFS_TYPE_FILE 2
    char    type;

    #define FILE_NAME_LEN 18 // last character for 0
    char    name[FILE_NAME_LEN+1];
} spfs_directory;
typedef struct { // the format of file item, its size is BLOCK_SIZE 
    int     next_file;              
    char    data[MAX_FILE_DATA_SIZE];
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


int spfs_mkfs(int device, int boot);
int spfs_existed(spfs_parameter *sb, int type, int *cdn, char *filename);
int spfs_open(spfs_parameter *sb, int cdn, char *filename);
#define SPFS_WRITE_APPEND   0
#define SPFS_WRITE_REWRITE  1
int spfs_write(spfs_parameter *sb, int dn, char data[], int size, int mode);
int spfs_read(spfs_parameter *sb, int dn, int offset, char data[], int size);

#endif // _SPFS_H_
