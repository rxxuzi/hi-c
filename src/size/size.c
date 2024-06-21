#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include "../util.h"

long calc_size(const char *path);

int size(const char *path) {
    char *fmt_size = getFileSize(calc_size(path));
    if (fmt_size == NULL) {
        c_R("Failed to get File size.\n");
        return 1;
    } else {
        c_G("Size : %s\n", fmt_size);
        free(fmt_size);
        return 0;
    }
}

long calc_size(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        perror("stat");
        return 0;
    }

    long totalSize = 0;

    if (S_ISREG(statbuf.st_mode)) {
        // ファイルの場合はそのサイズを返す
        totalSize = statbuf.st_size;
    } else if (S_ISDIR(statbuf.st_mode)) {
        // ディレクトリの場合は中身を再帰的に処理
        DIR *dir;
        struct dirent *entry;
        char fullPath[1024];

        dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            return 0;
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
            totalSize += calc_size(fullPath);
        }
        closedir(dir);
    }
    return totalSize;
}

void size_help() {
    c_B("Usage: size <path>\n");
    c_B("   path: The path to the file or directory.\n");
}
