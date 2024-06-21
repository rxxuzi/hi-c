#ifndef UTIL_H
#define UTIL_H

#include "util/console.h"
#include "util/fis.h"
int getFullPath(const char *path, char *fullPath, size_t size);
char* getFileSize(long size);

#endif //UTIL_H
