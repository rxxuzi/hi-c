// security.h
#ifndef SECURITY_H
#define SECURITY_H

typedef struct {
    char *fileName;
    unsigned char *buffer;
    unsigned int fileSize;
    char *hashStr;
    int status;
} FILEX;

char* SHA256(char* data);
char* SHA256F(char *file_path);
int readFX(FILEX* fx);
void freeFX(FILEX* fx);
#endif //SECURITY_H