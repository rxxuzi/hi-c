// cache.c
#include <stdlib.h>
#include "sync.h"
#include "../util/fis.h"
#include "flist.h"

int cache_write(FLIST *fl) {
    FILE *fp = fopen(SYNC_CACHE, "w");

    if (fp == NULL) {
        c_R("Error opening cache file for writing\n");
        return EXIT_FAILURE;
    }
    if (fl == NULL) {
        c_R("FLIST is NULL\n");
        return EXIT_FAILURE;
    }

    // write header
    fprintf(fp, "#SRC:%s\n",fl->src);
    fprintf(fp, "#LEN:%zu\n", fl->size);

    for (int i = 0; i < fl->size; i++) {
        fprintf(fp, "%s : %s\n", fl->hash[i], fl->path[i]);
    }
    fclose(fp);
    return EXIT_SUCCESS;
}

int cache(char *dir) {
    FLIST *fl = newFLIST(dir, true);
    initFLIST(fl);
    calcHash(fl);
    if (cache_write(fl) == EXIT_SUCCESS) {
        c_G("Cache written successfully : %s\n", dir);
    }
    freeFLIST(fl);
    return EXIT_SUCCESS;
}


