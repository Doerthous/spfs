##spfs v1.0
**特性**

1. 只有根目录
1. 能写入, 能读取, 不能修改
1. 文件名称有长度限制
1. 文件大小有限制
1. 不允许同名文件


**结构**

	1. 引导扇区(b), 512B
	2. 文件系统区(sb), 512B
	3. 目录位图区(dm), (1bit, 1024*32bit=4KB)
	4. 文件位图区(fm), (1bit, 1024*64bit=8KB)
	5. 目录区(d), (32B, 1024*32*32B=1MB)
	6. 文件区(f), (512B, 1024*64*512B=32MB)

**备注**

- 定义块为一扇区 
- 文件系统区包括, 目录数量(4B), 可用目录数量(4B), 文件数量(4B), 可用文件数量(4B), 目录位图块数量(4B), 文件位图块数量(4B), 目录块数量(4B), 文件块数量(4B)
- 位图区对应相应的区域的项目, 标记其使用情况
- 目录区包含目录项, 每个目录项大小为4B
- 目录项前4B表示该文件所占的第一个文件项的编号, 4B表示文件大小, 其余为文件名称
- 文件项大小为512B, 前4B表示下一数据区, 其余为文件数据
- 目录项, 文件项编号以1开始

**接口**

	1. name2directory(filename)
	2. name2file(filename)
	3. get_free_files(count)
	4. get_free_directory()
	5. get_system_block(sblock)
	6. set_system_block(sblock)
	7. get_directory_map(n, bitmap)
	8. set_directory_map(n, bitmap)
	9. get_file_map(n, bitmap)
	10. set_file_map(n, bitmap)
	11. get_directory(n, directory)
	12. set_directory(n, directory)
	13. get_file(n, file)
	14. set_file(n, file)

