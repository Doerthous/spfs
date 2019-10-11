## spfs

**结构**

    1. 引导扇区(b), 512B
    2. 文件系统区(sb), 512B
    3. 目录位图区(dm), (1bit, 1024*32bit=4KB)
    4. 文件位图区(fm), (1bit, 1024*64bit=8KB)
    5. 目录区(d), (32B, 1024*32*32B=1MB)
    6. 文件区(f), (512B, 1024*64*512B=32MB)
    
```
    block size: 512 B

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
```


**备注**

- 定义块为一扇区 
- 文件系统区包括, 目录数量(4B), 可用目录数量(4B), 文件数量(4B), 可用文件数量(4B), 目录位图块数量(4B), 文件位图块数量(4B), 目录块数量(4B), 文件块数量(4B)
- 位图区对应相应的区域的项目, 标记其使用情况
- 目录区包含目录项, 每个目录项大小为4B
- 目录项前4B表示该文件所占的第一个文件项的编号, 4B表示文件大小, 其余为文件名称
- 文件项大小为512B, 前4B表示下一数据区, 其余为文件数据
- 目录项, 文件项编号以1开始

**v1.0 特性**

- 只有根目录
- 能写入, 能读取, 不能修改
- 文件名称有长度限制
- 文件大小有限制
- 不允许同名文件

**v1.2 特性**

- 多层目录
