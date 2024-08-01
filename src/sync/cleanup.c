// cleanup.c
#include <stdio.h>
#include <windows.h>
#include "sync.h"

static void removeDirectory(const char *path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char fullPath[MAX_PATH];

    snprintf(fullPath, sizeof(fullPath), "%s\\*", path);
    hFind = FindFirstFile(fullPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
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

    if (!RemoveDirectory(path))  {
        c_R("Failed to remove directory: %s (Error: %lu)\n", path, GetLastError());
    }
}

int cleanup() {

    // Remove backup directory
    if (GetFileAttributes(SYNC_DIR) != INVALID_FILE_ATTRIBUTES) {
        removeDirectory(SYNC_DIR);
    } else {
        c_M("Backup directory does not exist: %s\n", SYNC_DIR);
    }

    // Remove cache file
    if (DeleteFile(SYNC_CACHE)) {
        c_G("Deleted cache file: %s\n", SYNC_CACHE);
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            c_M("Cache file does not exist: %s\n", SYNC_CACHE);
        } else {
            c_R("Failed to delete cache file: %s (Error: %lu)\n", SYNC_CACHE, error);
        }
    }

    c_G("Cleanup completed.\n");
    return EXIT_SUCCESS;
}