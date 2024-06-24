// main.c
#include <stdio.h>
#include <string.h>
#include "hash/hash.h"
#include "wdex/wdex.h"
#include "util/console.h"
#include "calc/calc.h"
#include "find/find.h"
#include "path/path.h"
#include "size/size.h"
#include "stat/stat.h"
#include "sync/sync.h"
#include "system.h"
#include <stdlib.h>

void help() {
    printf("Hi-C ! the command-line utilities! \n");
    printf("Usage: ");
    c_Y("hi-c ");
    c_C("[command] [arguments]\n\n");
    printf("Available commands:\n");

    printf("  hash      Calculate the hash of a file\n");
    printf("  wdex      Add a path to Windows Defender exclusion list\n");
    printf("  calc      Perform arithmetic operations\n");
    printf("  find      Find files or directories containing a specific term\n");
    printf("  path      Manage the system and user PATH variables\n");
    printf("  size      Get the size of a file or directory\n");
    printf("  stat      Monitor system resources like RAM, CPU, Disk, and Heap usage\n");
    printf("  sync      Backup, restore, and sync files across directories\n");
    printf("  help      Show this help message\n");
    printf("  version   Show the current version of HI-C\n");
    printf("\n");
    printf("For more information on a specific command, run -> ");
    c_Y("hi-c "); c_C("[command] -h\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        help();
        return 1;
    }

    if (argc > 1) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "version") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("HI-C Version : %s", HIC_VERSION);
            exit(0);
        }
        else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0|| strcmp(argv[1], "-?") == 0) {
            help();
            exit(0);
        }

        if (strcmp(argv[1], "hash") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                hash_help();
            } else {
                hash(argc, argv);
            }
        }
        else if (strcmp(argv[1], "wdex") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                wdex_help();
            } else if (argc == 3) {
                return wdex(argv[2]);
            } else {
                wdex_help();
                return -1;
            }
        }
        else if (strcmp(argv[1], "calc") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                calc_help();
            } else if (argc == 3) {
                calcp(argv[2]);
            } else {
                calc_help();
                return -1;
            }
        }
        else if (strcmp(argv[1], "find") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                find_help();
            } else if (argc == 3) {
                find(argv[2]);
            } else {
                find_help();
                return -1;
            }
        }
        else if (strcmp(argv[1], "path") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                path_help();
            } else if (argc >= 3) {
                path(argc, argv);
            } else {
                path_help();
                return -1;
            }
        }
        else if (strcmp(argv[1], "size") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                size_help();
                return 0;
            }
            if (argc == 3) {
                return size(argv[2]);
            } else {
                size_help();
                return -1;
            }
        }
        else if (strcmp(argv[1], "stat") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0) {
                stat_help();
            } else  {
                return stats(argc, argv);
            }
        }
        else if (strcmp(argv[1], "sync") == 0) {
            if (argc == 3 && strcmp(argv[2], "-h") == 0 || strcmp(argv[2], "--help") == 0) {
                sync_help();
                return -1;
            } else {
                sync(argc, argv);
            }
        }
        else {
            c_R("Invalid command\n");
            c_M("Type \"%s -h\" for help\n", argv[0]);
        }
    }
    return 0;
}
