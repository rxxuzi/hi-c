// cache.c
#include <stdlib.h>
#include "sync.h"
#include "../util/fis.h"
#include "flist.h"

int cache(char *dir) {
    FLIST *fl = newFLIST(dir, true);
    initFLIST(fl);
    calcHash(fl);
    if (writeFLIST(fl, SYNC_CACHE) == EXIT_SUCCESS) {
        c_G("Cache written successfully : %s\n", dir);
    }
    showFLIST(fl);
    freeFLIST(fl);
    return EXIT_SUCCESS;
}


