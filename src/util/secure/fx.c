// fx.c
#include "../security.h"
#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 2048

int readFX(FILEX* fx) {
    FILE *file = fopen(fx->fileName, "rb");
    if (!file) {
        perror("Could not open file");
        fx->status = 1;
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    fx->buffer = (unsigned char*)malloc(fileSize);
    if (!fx->buffer) {
        perror("Memory allocation failed.");
        fclose(file);
        fx->status = 3;
        return 3;
    }

    size_t bytesRead;
    size_t totalBytesRead = 0;
    while ((bytesRead = fread(fx->buffer + totalBytesRead, 1, BUFFER_SIZE, file)) > 0) {
        totalBytesRead += bytesRead;
    }

    fx->fileSize = totalBytesRead;

    fclose(file);
    fx -> status = 0;
    return 0;
}

void freeFX(FILEX* fx) {
    if (fx != NULL) {
        if (fx->buffer != NULL) {
            free(fx->buffer);
            fx->buffer = NULL;
        }
        if (fx->hashStr != NULL) {
            free(fx->hashStr);
            fx->hashStr = NULL;
        }
    }
}
