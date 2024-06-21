// io.c
#include <windows.h>
#include <stdio.h>
#include "../util.h"

#define K 1024

int getFullPath(const char *path, char *fullPath, size_t size) {
    if (GetFullPathName(path, (DWORD)size, fullPath, NULL) == 0) {
        c_R("Error getting full path: %lu\n", GetLastError());
        fullPath[0] = '\0';
        return -2;
    }
    return 0;
}

char* getFileSize(long size) {
    double convertedSize;
    char unit[3];

    if (size < K) {
        convertedSize = size;
        snprintf(unit, sizeof(unit), "B");
    } else if (size < K * K) {
        convertedSize = size / 1024.0;
        snprintf(unit, sizeof(unit), "KB");
    } else if (size < K * K * K) {
        convertedSize = size / (1024.0 * K);
        snprintf(unit, sizeof(unit), "MB");
    } else {
        convertedSize = size / (1024.0 * K * K);
        snprintf(unit, sizeof(unit), "GB");
    }

    char* result = (char*)malloc(10);
    if (result == NULL) return NULL;
    snprintf(result, 10, "%.2f%s", convertedSize, unit);
    return result;
}

int isSameFile(const char *path1, const char *path2) {
    HANDLE file1 = CreateFile(path1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file1 == INVALID_HANDLE_VALUE) {
        c_R("Error opening file %s: %lu\n", path1, GetLastError());
        return -1;
    }

    HANDLE file2 = CreateFile(path2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file2 == INVALID_HANDLE_VALUE) {
        c_R("Error opening file %s: %lu\n", path2, GetLastError());
        CloseHandle(file1);
        return -1;
    }

    BY_HANDLE_FILE_INFORMATION info1, info2;
    if (!GetFileInformationByHandle(file1, &info1)) {
        c_R("Error getting file information for %s: %lu\n", path1, GetLastError());
        CloseHandle(file1);
        CloseHandle(file2);
        return -1;
    }

    if (!GetFileInformationByHandle(file2, &info2)) {
        c_R("Error getting file information for %s: %lu\n", path2, GetLastError());
        CloseHandle(file1);
        CloseHandle(file2);
        return -1;
    }

    CloseHandle(file1);
    CloseHandle(file2);
    return CompareFileTime(&info1.ftLastWriteTime, &info2.ftLastWriteTime) == 0;
}