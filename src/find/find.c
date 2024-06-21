#include "../util.h"
#include "find.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void find_help() {
    c_B("Usage: find <path>\n");
}

int find(const char *str) {
    const char *path = "."; // 現在のディレクトリ
    FIS *fs = newFIS(path, true); // 再帰的に検索
    if (fs == NULL) {
        c_R("Failed to find\n");
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < fs->size; ++i) {
        if (strstr(fs->filepaths[i], str) != NULL) {
            printf("%s\n", fs->filepaths[i]);
        }
    }
    FISDestroy(fs);
    return EXIT_SUCCESS;
}
