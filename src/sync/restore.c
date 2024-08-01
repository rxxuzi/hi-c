// restore.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "flist.h"
#include "sync.h"

static void removeDirectory(const char *path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char fullPath[MAX_PATH];

    snprintf(fullPath, sizeof(fullPath), "%s\\*", path);
    hFind = FindFirstFile(fullPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        c_R("No files found in directory: %s\n", path);
        return;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            snprintf(fullPath, sizeof(fullPath), "%s\\%s", path, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                removeDirectory(fullPath);
            } else {
                if (!DeleteFile(fullPath)) {
                    c_R("Failed to delete file: %s (Error: %lu)\n", fullPath, GetLastError());
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    if (!RemoveDirectory(path)) {
        c_R("Failed to remove directory: %s (Error: %lu)\n", path, GetLastError());
    }
}

static void copyDirectory(const char *src, const char *dst) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char srcPath[MAX_PATH];
    char dstPath[MAX_PATH];

    snprintf(srcPath, sizeof(srcPath), "%s\\*", src);
    hFind = FindFirstFile(srcPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        c_R("No files found in source directory: %s\n", src);
        return;
    }

    if (!CreateDirectory(dst, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        c_R("Failed to create destination directory: %s (Error: %lu)\n", dst, GetLastError());
        return;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            snprintf(srcPath, sizeof(srcPath), "%s\\%s", src, findFileData.cFileName);
            snprintf(dstPath, sizeof(dstPath), "%s\\%s", dst, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                copyDirectory(srcPath, dstPath);
            } else {
                if (!CopyFile(srcPath, dstPath, FALSE)) {
                    c_R("Failed to copy file: %s (Error: %lu)\n", dstPath, GetLastError());
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

int restore(const char *targetDir) {

    FLIST *cacheList = readFlist(SYNC_CACHE);
    if (!cacheList) {
        c_R("Failed to read cache file: %s\n", SYNC_CACHE);
        return EXIT_FAILURE;
    }

    c_G("Target : %s\n", targetDir);

    DWORD attrs = GetFileAttributes(SYNC_DIR);
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        c_R("Backup directory does not exist: %s\n", SYNC_DIR);
        freeFLIST(cacheList);
        return EXIT_FAILURE;
    }

    removeDirectory(targetDir);

    if (!CreateDirectory(targetDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        c_R("Failed to create target directory: %s (Error: %lu)\n", targetDir, GetLastError());
        freeFLIST(cacheList);
        return EXIT_FAILURE;
    }

    copyDirectory(SYNC_DIR, targetDir);

    c_G("Restore operation completed.\n");

    freeFLIST(cacheList);
    return EXIT_SUCCESS;
}