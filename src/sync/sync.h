#ifndef SYNC_H
#define SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <stdio.h>
#include "../util/security.h"
#include "../util.h"
#include <stdbool.h>

#define SYNC_DIR "hi-backup"
#define SYNC_CACHE "hi-c.cache.txt"

#define BUFFER_SIZE 8192
void sync_help();
int sync(int argc, char **argv);
int cache(char *dir);
int diff(char *src, char *dst);
void restore(char *dir);
int watch(char *dir);

#ifdef __cplusplus
}
#endif


#endif //SYNC_H
