// diff.c
#include "flist.h"
#include "sync.h"

static long af = 0; // [+]
static long df = 0; // [-]
static long cf = 0; // [!]

static char* rmTopDir(char *path) {
    if (path == NULL) return NULL;
    char *slash = strrchr(path, '/');
    char *backslash = strrchr(path, '\\');
    if (!slash && !backslash) return path;
    char *firstDelimiter = slash > backslash ? slash : backslash;
    if (!firstDelimiter) return path; // トップディレクトリがない場合、元のパスを返す
    return firstDelimiter + 1;
}

int diff_fl(FLIST *src, FLIST *dst) {
    if (!src || !dst) {
        fprintf(stderr, "Invalid FLIST input.\n");
        return -1;
    }
    // srcにあってdstにないものを見つける
    for (size_t i = 0; i < src->size; i++) {
        bool found = false;
        char *srcPath = rmTopDir(src->path[i]);
        if (!srcPath) {
            c_R("Error: Invalid src path after rmTopDir\n");
            continue;
        }
        for (size_t j = 0; j < dst->size; j++) {
            char *dstPath = rmTopDir(dst->path[j]);
            if (strcmp(srcPath, dstPath) == 0) {
                found = true;
                if (!src->hash[i]) {
                    c_R("[?] %s\n", i, src->path[i]);
                    continue;
                }
                if (!dst->hash[j]) {
                    c_R("[?] %s\n", j, dst->path[j]);
                    continue;
                }
                if (strcmp(src->hash[i], dst->hash[j]) != 0) {
                    c_C("[!] %s\n", srcPath);
                    cf ++;
                }
                break;
            }
        }
        if (!found) {
            c_C("[+] %s\n", srcPath);
            af ++;
        }
    }

    for (size_t j = 0; j < dst->size; j++) {
        bool found = false;
        char *dstPath = rmTopDir(dst->path[j]);
        for (size_t i = 0; i < src->size; i++) {
            char *srcPath = rmTopDir(src->path[i]);
            if (strcmp(dstPath, srcPath) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            c_C("[-] %s\n", dstPath);
            df ++;
        }
    }

    if (af == 0 && df == 0 && cf == 0) {
        c_G("No differences found.\n");
    } else {
        c_G("Found %ld differences.\n", af + df + cf);
    }

    return 0;
}

int diff(char *src, char *dst) {
    FLIST *sfl = newFLIST(src, true);
    FLIST *dfl = newFLIST(dst, true);
    if (!sfl || !dfl) {
        c_R("NULL ERR");
        return -1;
    }
    initFLIST(sfl);
    initFLIST(dfl);
    calcHash(sfl);
    calcHash(dfl);
    diff_fl(sfl, dfl);
    freeFLIST(sfl);
    freeFLIST(dfl);

    return 0;
}
