// wdex.c
#include <windows.h>
#include <stdio.h>
#include "../util.h"

void wdex_help() {
    c_B("Usage: wdex <path>\n");
    c_B("<path>     Path to file or directory.\n");
    c_B("           If you want to specify the current directory, use '.'.\n");
}

int elevator(const char *path, DWORD fileType) {
    if (path[0] == '\0') {
        c_R("Failed to get full path.\n");
        return -1;
    }

    char command[1024];

    if (fileType == FILE_ATTRIBUTE_DIRECTORY) {
        snprintf(command, sizeof(command), "powershell -Command \"Add-MpPreference -ExclusionPath '%s'\"", path);
    } else {
        snprintf(command, sizeof(command), "powershell -Command \"Add-MpPreference -ExclusionProcess '%s'\"", path);
    }

    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = "runas";
    sei.lpFile = "powershell.exe";
    sei.lpParameters = command;
    sei.hwnd = NULL;
    sei.nShow = SW_SHOWNORMAL;
    sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;

    if (!ShellExecuteEx(&sei)) {
        DWORD error = GetLastError();
        if (error == ERROR_CANCELLED) {
            c_R("User refused to elevate.\n");
        } else {
            c_R("ShellExecuteEx failed.\n");
        }
        return -1;
    }

    // プロセスが終了するのを待つ
    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);

    return 0;
}

int wdex(const char *path) {
    if (strcmp(path, "-h") == 0) {
        wdex_help();
        return 0;
    }
    char fullPath[MAX_PATH];

    if (strcmp(path, ".") == 0) {
        // fullpath is this folder
        if (GetCurrentDirectory(MAX_PATH, fullPath) == 0) {
            c_R("Error getting current directory: %lu\n", GetLastError());
            return -1;
        }
        c_Y("Current directory: %s\n", fullPath);
        DWORD type = GetFileAttributes(fullPath);
        return elevator(fullPath, type);
    }

    if (getFullPath(path, fullPath, MAX_PATH) != 0) {
        return -1;
    }

    if (GetFileAttributes(fullPath) == INVALID_FILE_ATTRIBUTES) {
        c_R("File not found.\n");
        return -1;
    }

    DWORD type = GetFileAttributes(fullPath);
    return elevator(fullPath, type);
}