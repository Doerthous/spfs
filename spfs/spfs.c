#include "spfs.h"

#define DIRECTORY_SIZE              sizeof(spfs_directory)

static char block_buffer[MAX_BLOCK_SIZE];
static spfs_file file_cache;

/*
    SPFS 依赖的工具函数
*/
#ifndef SPFS_STRNCMP
static int spfs_strncmp(const char *str1, const char *str2, unsigned int n){
    for (; *str1 && *str2 && n; ++str1, ++str2, --n){
        if(*str1 != *str2) {
            return *str1 - *str2;
        }
    }
    if (*str1 && !*str2) {
        return 1;
    }
    if (!*str1 && *str2) {
        return -1;
    }
    return 0;
}
#define SPFS_STRNCMP(str1, str2, n) \
        spfs_strncmp((char *)(str1), (char *)(str2), (unsigned int)(n))
#endif // SPFS_STRNCMP

#ifndef SPFS_MEMSET
static void spfs_memset(char *mem, char val, unsigned int size) {
    for (; size; ++mem, --size) {
        *mem = val;
    }
}
#define SPFS_MEMSET(mem, val, size) \
        spfs_memset((char *)(mem), (char)(val), (unsigned int)(size))
#endif // SPFS_MEMSET

#ifndef SPFS_MEMCPY
static void spfs_memcpy(char *dest, char *src, unsigned int size) {
    for (; size; ++dest, ++src, --size) {
        *dest = *src;
    }
}
#define SPFS_MEMCPY(dest, src, size) \
        spfs_memcpy((char *)(dest), (char *)(src), (unsigned int)(size))
#endif // SPFS_MEMCPY

#ifndef SPFS_SET_BIT
static void spfs_set_bit(char *bm, int len, int i)  {
    const int byte = sizeof(char) * 8;
    if (i < len) {
        bm[i / byte] |= (1 << (i % byte));
    }
}
#define SPFS_SET_BIT(bm, len, i) \
        spfs_set_bit((bm), (len), (i))
#endif // SPFS_SET_BIT

#ifndef SPFS_CLEAR_BIT
static void spfs_clear_bit(char *bm, int len, int i) {
    const int byte = sizeof(char) * 8;
    if (i < len) {
        bm[i / byte] &= ~(1 << (i % byte));
    }
}
#define SPFS_CLEAR_BIT(bm, len, i) \
        spfs_clear_bit((bm), (len), (i))
#endif // SPFS_CLEAR_BIT

#ifndef SPFS_TEST_BIT
static int spfs_test_bit(char *bm, int len, int i){
    const int byte = sizeof(char) * 8;
    if (i < len) {
        int byte = sizeof(char) * 8;
        return bm[i / byte] & (1 << (i % byte));
    }
    return 0;
}
#define SPFS_TEST_BIT(bm, len, i) \
        spfs_test_bit((bm), (len), (i))
#endif // SPFS_TEST_BIT


/*
    与 device 层的接口
*/
#if (!defined(SPFS_READ_SECTOR) || !defined(SPFS_WRITE_SECTOR))
static int rw_sector(int a, int b, char *c, int d) {}
#endif // SPFS_READ_SECTOR || SPFS_WRITE_SECTOR
#ifndef SPFS_READ_SECTOR
#define SPFS_READ_SECTOR(dev, sn, buf, size) \
        rw_sector((dev), (sn), (buf), (size))
#endif // SPFS_READ_SECTOR
#ifndef SPFS_WRITE_SECTOR
#define SPFS_WRITE_SECTOR(dev, sn, buf, size) \
        rw_sector((dev), (sn), (buf), (size))
#endif // SPFS_WRITE_SECTOR

static void rw_block(int device, int bn, char *buf, int rw) {
    spfs_parameter sb;
    int sb_sec = 0;
    read_sector(device, 1, (char *)&sb, sizeof(sb));
    if (SPFS_STRNCMP(sb.signature, "spfs", 4)) {
        read_sector(device, 2, (char *)&sb, sizeof(sb));
        sb_sec = 1;
    }
    if (bn > 0) { // 
        if (rw) {
            SPFS_READ_SECTOR(device, bn*sb.sector_count_per_block+sb_sec, 
            buf, get_sector_size(device));
        } else {
            SPFS_WRITE_SECTOR(device, bn*sb.sector_count_per_block+sb_sec, 
            buf, get_sector_size(device));
        }
        
    }
}

#define read_block(dev, bn, buf) \
        rw_block((dev), (bn), (buf), 1)
#define write_block(dev, bn, buf) \
        rw_block((dev), (bn), (buf), 0)

/*
    缓存机制
*/
#define SPFS_USE_CACHE
#ifdef SPFS_USE_CACHE
static struct {
    int  dmbc_dev;
    int  dmbc_dmbn;
    char dir_map_blk_cache[MAX_BLOCK_SIZE];

    int  fmbc_dev;
    int  fmbc_fmbn;
    char file_map_blk_cache[MAX_BLOCK_SIZE];

    int  dbc_dev;
    int  dbc_dbn;
    char dir_blk_cache[MAX_BLOCK_SIZE]; 

    char dmbc_dirty:1;
    char fmbc_dirty:1;
    char dbc_dirty:1;
} cache;
static void spfs_cache_flush() {
    if (cache.dmbc_dirty) {
        write_block(cache.dmbc_dev, cache.dmbc_dmbn, cache.dir_map_blk_cache);
        cache.dmbc_dirty = 0;
    }
    if (cache.fmbc_dirty) {
        write_block(cache.fmbc_dev, cache.fmbc_fmbn, cache.file_map_blk_cache);
        cache.fmbc_dirty = 0;
    }
    if (cache.dbc_dirty) {
        write_block(cache.dbc_dev, cache.dbc_dbn, cache.dir_blk_cache);
        cache.dbc_dirty = 0;
    }
}
#endif // SPFS_USE_CACHE


/*
    SPFS 函数
*/
void get_system_block(int device, spfs_parameter *sb) {
    read_block(device, 1, block_buffer);
    sb->device = device;
    SPFS_MEMCPY(sb, block_buffer, sizeof(spfs_parameter));
}
void set_system_block(spfs_parameter *sb) {
    SPFS_MEMCPY(block_buffer, sb, sizeof(spfs_parameter));
    write_block(sb->device, 1, block_buffer);
#ifdef SPFS_USE_CACHE
    spfs_cache_flush();
#endif // 
}

void get_directory_map_block(spfs_parameter *sb, int dmn, char *block) {
    int start = 1 + dmn;
#ifndef SPFS_USE_CACHE
    read_block(sb->device, start, block);
#else
    /** cache write back */
    if (cache.dmbc_dev != sb->device || cache.dmbc_dmbn != dmn) {
        /** write cache back to device */
        if (cache.dmbc_dirty) {
            write_block(cache.dmbc_dev, cache.dmbc_dmbn, cache.dir_map_blk_cache);
        }
        /** read new block to cache */
        read_block(sb->device, start, cache.dir_map_blk_cache);
        cache.dmbc_dev = sb->device;
        cache.dmbc_dmbn = start;
        cache.dmbc_dirty = 0;
    }
    /** copy to blcok */
    SPFS_MEMCPY(block, cache.dir_map_blk_cache, 
        sizeof(cache.dir_map_blk_cache));
#endif // SPFS_USE_CACHE
}

void set_directory_map_block(spfs_parameter *sb, int dmn, char *block) {
    int start = 1 + dmn;
#ifndef SPFS_USE_CACHE
    write_block(sb->device, 1 + dmn, block);
#else
    /** cache write back */
    if (cache.dmbc_dev != sb->device || cache.dmbc_dmbn != dmn) {
        /** write cache back to device */
        if (cache.dmbc_dirty) {
            write_block(cache.dmbc_dev, cache.dmbc_dmbn, cache.dir_map_blk_cache);
        }
        /** cache block info */
        //write_block(sb->device, start, block);
        cache.dmbc_dev = sb->device;
        cache.dmbc_dmbn = start;
        //cache.dmbc_dirty = 0;
    }
    /** cache data */
    SPFS_MEMCPY(cache.dir_map_blk_cache, block, 
        sizeof(cache.dir_map_blk_cache));
    cache.dmbc_dirty = 1;
#endif // SPFS_USE_CACHE
}

void get_file_map(spfs_parameter *sb, int fmn, char *block) {
    int start = 1 + sb->directory_map_block_count + fmn;
#ifndef SPFS_USE_CACHE
    read_block(sb->device, start, block);
#else
    /** cache write back */
    if (cache.fmbc_dev != sb->device || cache.fmbc_fmbn != start) {
        /** write cache back to device */
        if (cache.fmbc_dirty) {
            write_block(cache.fmbc_dev, cache.fmbc_fmbn, cache.file_map_blk_cache);
        }
        /** read new block to cache */
        read_block(sb->device, start, cache.file_map_blk_cache);
        cache.fmbc_dev = sb->device;
        cache.fmbc_fmbn = start;
        cache.fmbc_dirty = 0;
    }
    /** copy to blcok */
    SPFS_MEMCPY(block, cache.file_map_blk_cache, 
        sizeof(cache.file_map_blk_cache));
#endif //
}

void set_file_map(spfs_parameter *sb, int fmn, char *block) {
    int start = 1 + sb->directory_map_block_count + fmn;
#ifndef SPFS_USE_CACHE
    write_block(sb->device, start, block);
#else
    /** cache write back */
    if (cache.fmbc_dev != sb->device || cache.fmbc_fmbn != start) {
        /** write cache back to device */
        if (cache.fmbc_dirty) {
            write_block(cache.fmbc_dev, cache.fmbc_fmbn, cache.file_map_blk_cache);
        }
        /** write data to device */
        //write_block(sb->device, start, block);
        cache.fmbc_dev = sb->device;
        cache.fmbc_fmbn = start;
    }
    /** cache data */
    SPFS_MEMCPY(cache.file_map_blk_cache, block, 
        sizeof(cache.file_map_blk_cache));
    cache.fmbc_dirty = 1;
#endif //
}

static void get_directory_block(spfs_parameter *sb, int dn, char *block) {
    int start = 1 + sb->directory_map_block_count + 
            sb->file_map_block_count + dn;
#ifndef SPFS_USE_CACHE
    read_block(sb->device, start, block);
#else
    /** cache write back */
    if (cache.dbc_dev != sb->device || cache.dbc_dbn != start) {
        /** write cache back to device */
        if (cache.dbc_dirty) {
            write_block(cache.dbc_dev, cache.dbc_dbn, cache.dir_blk_cache);
        }
        /** read new block to cache */
        read_block(sb->device, start, cache.dir_blk_cache);
        cache.dbc_dev = sb->device;
        cache.dbc_dbn = start;
        cache.dbc_dirty = 0;
    }
    /** copy to blcok */
    SPFS_MEMCPY(block, cache.dir_blk_cache, 
        sizeof(cache.dir_blk_cache));
#endif //
}

static void set_directory_block(spfs_parameter *sb, int dn, char *block) {
    int start = 1 + sb->directory_map_block_count + 
            sb->file_map_block_count + dn;
#ifndef SPFS_USE_CACHE
    write_block(sb->device, start, block);
#else
    /** cache write back */
    if (cache.dbc_dev != sb->device || cache.dbc_dbn != start) {
        /** write cache back to device */
        if (cache.dbc_dirty) {
            write_block(cache.dbc_dev, cache.dbc_dbn, cache.dir_blk_cache);
        }
        /** write data to device */
        //write_block(sb->device, start, block);
        cache.dbc_dev = sb->device;
        cache.dbc_dbn = start;
    }
    /** cache data */
    SPFS_MEMCPY(cache.dir_blk_cache, block, 
        sizeof(cache.dir_blk_cache));
    cache.dbc_dirty = 1;
#endif //
}

void spfs_get_directory(spfs_parameter *sb, int dn, spfs_directory *d) {
    --dn; // number -> index
    int dir_blk_num = (dn/(sb->block_size/DIRECTORY_SIZE))+1;
    int dir_idx = dn%(sb->block_size/DIRECTORY_SIZE);
    get_directory_block(sb, dir_blk_num, block_buffer);
    spfs_directory *dirs = (spfs_directory *)block_buffer;
    SPFS_MEMCPY(d, (dirs+dir_idx), DIRECTORY_SIZE);
}
void spfs_set_directory(spfs_parameter *sb, int dn, spfs_directory *d) {
    --dn; // number -> index
    int dir_blk_num = (dn/(sb->block_size/DIRECTORY_SIZE))+1;
    int dir_idx = dn%(sb->block_size/DIRECTORY_SIZE);
    get_directory_block(sb, dir_blk_num, block_buffer);
    spfs_directory *dirs = (spfs_directory *)block_buffer;
    SPFS_MEMCPY(&dirs[dir_idx], d, DIRECTORY_SIZE);
    set_directory_block(sb, dir_blk_num, block_buffer);
}
void spfs_get_file(spfs_parameter *sb, int fn, spfs_file *f) {
    int start = 1 + sb->directory_map_block_count + 
        		sb->file_map_block_count + 
        		sb->directory_block_count + fn;
    read_block(sb->device, start, (char *)f);
}
void spfs_set_file(spfs_parameter *sb, int fn, spfs_file *f) {
    int start = 1 + sb->directory_map_block_count + 
        		sb->file_map_block_count + 
        		sb->directory_block_count + fn;
    write_block(sb->device, start, (char *)f);
}

// file organization module
/*
    获取 file_count 个可用的 file，并将他们串联，
    然后返回它们头节点的编号。
    return 的编号从 1 编号到 file_block_count。
 */
int get_free_files(spfs_parameter *sb, int file_count) {
    // 获取 system block，判断剩余的 file block 是否满足 file_count
    if (sb->free_file_count < file_count)
        return 0;
    int block_size = sb->block_size;
    char *file_map = block_buffer;
    sb->free_file_count -= file_count;
    spfs_file *file = &file_cache;
    SPFS_MEMSET(file, 0, block_size);
    int bit_count = block_size * 8;
    // 串联选中的 free file block
    int file_head = 0;
    for (int j = 0; j < sb->file_map_block_count && file_count; ++j) {
        get_file_map(sb, j+1, file_map);
        int last_file_number = 0;
        int is_file_map_updated = 0;
        for (int i = 0; i < bit_count && file_count; ++i) {
            int curr_file_number = j * bit_count + i + 1;
            if (!SPFS_TEST_BIT(file_map, bit_count, i)) {
                SPFS_SET_BIT(file_map, bit_count, i);
                is_file_map_updated = 1;
                // 记录头节点
                if (file_head == 0) { 
                    file_head = curr_file_number;
                }
                // 串联其他节点
                if (last_file_number != 0) {
                    file->next_file = curr_file_number;
                    spfs_set_file(sb, last_file_number, file); // 将上一个 file 与当前 file 串联
                }
                last_file_number = curr_file_number;
                //get_file(device, last_file_number, file);
                --file_count;
            }
        }
        if (is_file_map_updated) {
            set_file_map(sb, j+1, file_map);
        }
    }
    set_system_block(sb);
    return file_head;
}

int get_free_directory(spfs_parameter *sb) { 
    // 获取 system block，判断是否有剩余的 directory item
    if (sb->free_directory_count <= 0)
        return 0;
    //char directory[BLOCK_SIZE];
    int block_size = sb->block_size;
    char *directory_map = block_buffer;
    int directory_number = 0;
    int is_directory_map_updated = 0;
    int bit_count = block_size * 8;
    // 遍历 directory map 寻找可用的 directory item 的编号
    for (int j = 0; j < sb->directory_map_block_count && !directory_number; ++j) {
        get_directory_map_block(sb, j + 1, directory_map);
        for (int i = 0; i < bit_count && !directory_number; ++i) {
            int curr_directory_number = j * bit_count + i + 1;
            if (!SPFS_TEST_BIT(directory_map, bit_count, i)) {
                SPFS_SET_BIT(directory_map, bit_count, i);
                is_directory_map_updated = 1;
                directory_number = curr_directory_number;
                --sb->free_directory_count;
            }
        }
        // 如果已找到一个 directory item 则返回
        if (is_directory_map_updated) {
            set_directory_map_block(sb, j + 1, directory_map);
            break;
        }
    }
    set_system_block(sb);
    return directory_number;
}

int get_directory_by_filename(spfs_parameter *sb, char *fname, spfs_directory *dir) {
    char *directory = block_buffer;//[BLOCK_SIZE];
    int block_size = sb->block_size;
    char *directory_map = block_buffer;//[BLOCK_SIZE];
    // 遍历 directory map 找到有效的 directory
    int bit_count = block_size * 8;
    for (int j = 0; j < sb->directory_map_block_count; ++j) {
        get_directory_map_block(sb, j + 1, directory_map);
        for (int i = 0; i < bit_count; ++i) {
            int directory_number = j * bit_count + i + 1;
            if (SPFS_TEST_BIT(directory_map, bit_count, i)) {
                --directory_number;
                int dn1 = directory_number / DIRECTORY_SIZE;
                int dn2 = directory_number % DIRECTORY_SIZE;
                get_directory_block(sb, dn1 + 1, directory);
                spfs_directory *ds = (spfs_directory *)directory;
                if (ds[dn2].file_head) {
                    if (!SPFS_STRNCMP(fname, ds[dn2].name, FILE_NAME_LEN)) {
                        spfs_get_directory(sb, directory_number+1, dir);
                        return directory_number+1;
                    }
                }
                get_directory_map_block(sb, j + 1, directory_map);
            }
        }
    }
    return 0;
}

int get_file_by_filename(spfs_parameter *sb, char *fname, spfs_file *file) {
    spfs_directory dir;
    int ret;
    if ((ret = get_directory_by_filename(sb, fname, &dir)) != 0) {
        spfs_get_file(sb, dir.file_head, file);
    }
    return ret;
}

