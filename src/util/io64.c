#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../util.h"

#define K (int64_t)1024
#define M ((int64_t)K * K)
#define G ((int64_t)K * M)
#define T ((int64_t)K * G)

char* getFileSize64(int64_t size) {
    double convertedSize;
    const char* unit;

    if (size < K) {
        convertedSize = (double)size;
        unit = "B";
    } else if (size < M) {
        convertedSize = (double)size / K;
        unit = "KB";
    } else if (size < G) {
        convertedSize = (double)size / M;
        unit = "MB";
    } else if (size < T) {
        convertedSize = (double)size / G;
        unit = "GB";
    } else {
        convertedSize = (double)size / T;
        unit = "TB";
    }

    char* result = (char*)malloc(16);  // Increased buffer size for larger numbers
    if (result == NULL) return NULL;
    snprintf(result, 16, "%.2f%s", convertedSize, unit);
    return result;
}