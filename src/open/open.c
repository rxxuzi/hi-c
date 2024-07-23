#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>

#define MAX_PATH_LENGTH 260

void open_help() {
    printf("Usage: hi-c open <path1> [path2] [path3] ...\n");
    printf("Opens the specified directories in Windows Explorer and brings them to the foreground.\n");
    printf("\nOptions:\n");
    printf("  --help    Display this help message\n");
}

char* get_absolute_path(const char* relative_path) {
    static char abs_path[MAX_PATH_LENGTH];
    if (_fullpath(abs_path, relative_path, MAX_PATH_LENGTH) != NULL) {
        return abs_path;
    }
    return NULL;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == (DWORD)lParam) {
        SetForegroundWindow(hwnd);
        return FALSE;  // Stop enumerating
    }
    return TRUE;  // Continue enumerating
}

int open_directory(const char* path) {
    char* abs_path = get_absolute_path(path);
    if (abs_path == NULL) {
        printf("Error: Unable to resolve path '%s'\n", path);
        return 1;
    }

    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;  // We need the process handle
    sei.lpVerb = "open";
    sei.lpFile = "explorer.exe";
    sei.lpParameters = abs_path;
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteEx(&sei)) {
        printf("Directory opened successfully: %s\n", abs_path);

        // Wait for the process to be created
        WaitForInputIdle(sei.hProcess, 1000);

        // Get the process ID
        DWORD processId = GetProcessId(sei.hProcess);

        // Enumerate windows to find the one we just opened
        EnumWindows(EnumWindowsProc, (LPARAM)processId);

        CloseHandle(sei.hProcess);
        return 0;
    } else {
        DWORD error = GetLastError();
        printf("Error opening directory '%s'. Error code: %lu\n", abs_path, error);
        return 1;
    }
}

int open_dir(int argc, char **argv) {
    if (argc < 3) {
        printf("Error: No path specified.\n");
        open_help();
        return 1;
    }

    if (strcmp(argv[2], "--help") == 0) {
        open_help();
        return 0;
    }

    int success = 0;
    for (int i = 2; i < argc; i++) {
        if (open_directory(argv[i]) == 0) {
            success++;
        }
    }

    printf("Opened %d out of %d directories successfully.\n", success, argc - 2);
    return (success == argc - 2) ? 0 : 1;
}