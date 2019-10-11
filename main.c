#include <stdio.h>
#include "spsh/spsh.h"
#include "spdev/vdev.h"

void print_usage() {
    printf("Usage: \n");
    printf("    spfs -c type imagename\n");
    printf("        create a img file\n");
    printf("    spfs -m [b] imagename\n");
    printf("        make file system on imagename, b: set first sector as boot sector\n");
    printf("    spfs -i imagename filename [newname]\n");
    printf("        insert file to imagename\n");
    printf("    spfs -s imagename\n");
    printf("        shell\n\n");
    printf("    type: DTS1, W25Q16, W25Q64\n");
}

int main(int argc, char *args[]) {
    if (argc < 2) {
        print_usage();
    }
    else if (!strcmp(args[1], "-c")) { // spfs -c type imagename
        if (argc != 4) {
            print_usage();
        }
        else {
            if (!strncmp(args[2], "DTS1", 4)) {
                create_vdevice(args[3], 512, 32*1024);
                printf("%s has created\n", args[3]);
            } 
            else if (!strncmp(args[2], "W25Q16", 6)) {
                create_vdevice(args[3], 4096, 512);
                printf("%s has created\n", args[3]);
            }
            else if (!strncmp(args[2], "W25Q64", 6)) {
                create_vdevice(args[3], 4096, 2048);
                printf("%s has created\n", args[3]);
            }
            else {
                print_usage();
            }
        }
    }
    else if (!strcmp(args[1], "-m")) { // spfs -m [b] imagename
        int b = 0;
        char *img;
        if (argc == 3) {
            img = args[2];
        }
        else if (argc == 4 && !strcmp(args[2], "b")) {
            img = args[3];
            b = 1;
        }
        else {
            print_usage();
            return 0;
        }
        int d = open_vdevice(img);
        if (d != DEVICE_OPEN_FAILED) {
            spfs_mkfs(d, b);
            close_vdevice(d);
        }
        else {
            //
        }
    }
    else if (!strcmp(args[1], "-i")) { // spfs -i imagename filename [newname]
        if (argc > 3) {
            int d = open_vdevice(args[2]);
            if (d != DEVICE_OPEN_FAILED) {
                spfs_parameter sb;
                shell_env *sh_env = get_shell_env();
                get_system_block(d, &sh_env->fs_sys_blk);
                sh_env->ch2root();
                ist(argc-2, args+2);
                set_system_block(&sh_env->fs_sys_blk);
                close_vdevice(d);
            }
            else {
                printf("Open device %s failed!\n", args[2]);
            }
        }
        else {
            print_usage();
        }
    }
    else if (!strcmp(args[1], "-s")) { // spfs -d imagename
        if (argc == 3) {
            int d = open_vdevice(args[2]);
            if (d != DEVICE_OPEN_FAILED) {
                shell(d);
                close_vdevice(d);
            }
            else {
                printf("Open device %s failed!\n", args[2]);
            }
        }
        else {
            print_usage();
        }
    }
    else {
        print_usage();
    }
}
