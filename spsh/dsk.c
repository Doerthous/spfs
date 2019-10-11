#include "spsh.h"
#include "spio.h"

void dsk(int argc, char *args[]) {
	shell_env *sh_env = get_shell_env();
	printf("Has boot sector:        %s\n\n", sh_env->fs_sys_blk.has_boot_sector ? "YES" : "NO");
	printf("Directory map blocks:   %d\n", sh_env->fs_sys_blk.directory_map_block_count);
	printf("File map blocks:        %d\n", sh_env->fs_sys_blk.file_map_block_count);
	printf("Directory blocks:       %d\n", sh_env->fs_sys_blk.directory_block_count);
	printf("File blocks:            %d\n\n", sh_env->fs_sys_blk.file_block_count);
	printf("Directorys:             (%d/%d)\n", sh_env->fs_sys_blk.free_directory_count, sh_env->fs_sys_blk.directory_count);
	printf("Files:                  (%d/%d)\n", sh_env->fs_sys_blk.free_file_count, sh_env->fs_sys_blk.file_count);	
}