// hash.c
#include "../util.h"
#include "hash.h"
#include "../util/security.h"

void hash_help(void) {
    c_B("Usage: hash [option] <path>\n");
    c_B("Options:\n");
    c_B("  --str <string>      Hash a given string.\n");
    c_B("  --file <file_path>  Calculate file hash.\n");
    c_B("  --help              Show this help message.\n");
    c_B("  <path>              Calculate file hash.\n");
}

void hash(int argc, char **argv) {
    if (argc < 3) {
        hash_help();
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--str") == 0) {
            if (i + 1 < argc) {
                char *str = argv[i + 1];
                char *hash_str = SHA256(str);
                if (hash_str != NULL) {
                    c_G("String : %s\n", str);
                    c_G("Hash : %s\n", hash_str);
                    free(hash_str);
                }
            } else {
                printf("Error: --str requires a string argument.\n");
            }
            return;
        } else if (strcmp(argv[i], "--file") == 0) {
            if (i + 1 < argc) {
                char *file_path = argv[i + 1];
                char *hash_str = SHA256F(file_path);
                if (hash_str != NULL) {
                    c_G("File : %s\n", file_path);
                    c_G("Hash : %s\n", hash_str);
                    free(hash_str);
                }
            } else {
                printf("Error: --file requires a file path argument.\n");
            }
            return;
        }
    }

    // 何もない場合はファイルハッシュをとる
    if (argc == 3) {
        char *file_path = argv[2];
        char *hash_str = SHA256F(file_path);
        if (hash_str != NULL) {
            c_G("File : %s\n", file_path);
            c_G("Hash : %s\n", hash_str);
            free(hash_str);
        } else {
            printf("Error: Failed to calculate file hash.\n");
        }
        return;
    }
}