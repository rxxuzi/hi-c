// apth.c
#include <windows.h>
#include <stdio.h>
#include "../util.h"

#define MODE_SYSTEM 0
#define MODE_USER 1

// 既存の PATH に fullPath が含まれているかどうかをチェックする関数
BOOL isPathAlreadyIncluded(const char* currentPath, const char* fullPath) {
    const char* found = strstr(currentPath, fullPath);
    if (found == NULL) {
        return FALSE;  // fullPath が見つからない場合
    }

    // パスの境界を正しく検出する
    const char* end = found + strlen(fullPath);
    if ((found == currentPath || *(found - 1) == ';') && (*end == '\0' || *end == ';')) {
        return TRUE;
    }
    return FALSE;
}


// レジストリを更新する関数
int apth(const char* path, int mode) {
    char fullPath[MAX_PATH];
    if (!GetFullPathName(path, MAX_PATH, fullPath, NULL)) {
        c_R("Failed to get full path. Error: %lu\n", GetLastError());
        c_M("Make sure the path is correct and try again.\n");
        return -1;
    }

    HKEY hKey;
    LPCTSTR targetKey = (mode == MODE_SYSTEM) ? TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment") : TEXT("Environment");
    HKEY rootKey = (mode == MODE_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    LONG result = RegOpenKeyEx(rootKey, targetKey, 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        c_R("Unable to open registry key. Error: %lu\n", result);
        if (result == ERROR_ACCESS_DENIED) {
            c_M("You may not have the necessary administrative rights. Try running the application as an administrator.\n");
        } else {
            c_M("Check the registry path and key permissions.\n");
        }
        return -1;
    }

    char currentPath[32767];
    DWORD pathLen = sizeof(currentPath);
    if (RegQueryValueEx(hKey, TEXT("Path"), NULL, NULL, (LPBYTE)currentPath, &pathLen) != ERROR_SUCCESS) {
        c_R("Unable to read current PATH. Error: %lu\n", GetLastError());
        c_M("Ensure you have permissions to read the PATH variable and try again.\n");
        RegCloseKey(hKey);
        return -1;
    }

    if (!isPathAlreadyIncluded(currentPath, fullPath)) {
        snprintf(currentPath + strlen(currentPath), sizeof(currentPath) - strlen(currentPath), ";%s", fullPath);
        if (RegSetValueEx(hKey, TEXT("Path"), 0, REG_EXPAND_SZ, (LPBYTE)currentPath, (DWORD)(strlen(currentPath) + 1)) != ERROR_SUCCESS) {
            c_R("Unable to set new PATH. Error: %lu\n", GetLastError());
            c_M("Ensure you have permissions to modify the PATH variable and try again.\n");
            RegCloseKey(hKey);
            return -1;
        }
    } else {
        c_M("The specified path is already included in the PATH variable.\n");
    }

    RegCloseKey(hKey);
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    c_G("PATH updated successfully in %s mode.\n", (mode == MODE_SYSTEM) ? "system" : "user");
    return 0;
}

// 既存の PATH から指定された fullPath を削除する関数
BOOL removePath(char* currentPath, const char* fullPath) {
    char* pos = strstr(currentPath, fullPath);
    if (pos != NULL) {
        // パスが見つかった場合、そのパスをスキップして後ろを前に詰める
        const char* end = pos + strlen(fullPath);
        if (*end == ';') end++;  // セミコロンも削除
        memmove(pos, end, strlen(end) + 1);
        return TRUE;
    }
    return FALSE;
}

// 環境変数 PATH からパスを削除する関数
int dpth(const char* path, int mode) {
    char fullPath[MAX_PATH];
    if (!GetFullPathName(path, MAX_PATH, fullPath, NULL)) {
        c_R("Failed to get full path. Error: %lu\n", GetLastError());
        c_M("Make sure the path is correct and try again.\n");
        return -1;
    }

    HKEY hKey;
    LPCTSTR targetKey = (mode == MODE_SYSTEM) ? TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment") : TEXT("Environment");
    HKEY rootKey = (mode == MODE_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    if (RegOpenKeyEx(rootKey, targetKey, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        c_R("Unable to open registry key. Error: %lu\n", GetLastError());
        c_M("You may not have the necessary rights. Check your permissions.\n");
        return -1;
    }

    char currentPath[32767];
    DWORD pathLen = sizeof(currentPath);
    if (RegQueryValueEx(hKey, TEXT("Path"), NULL, NULL, (LPBYTE)currentPath, &pathLen) != ERROR_SUCCESS) {
        c_R("Unable to read current PATH. Error: %lu\n", GetLastError());
        c_M("Ensure you have permissions to read the PATH variable and try again.\n");
        RegCloseKey(hKey);
        return -1;
    }

    if (removePath(currentPath, fullPath)) {
        if (RegSetValueEx(hKey, TEXT("Path"), 0, REG_EXPAND_SZ, (LPBYTE)currentPath, (DWORD)(strlen(currentPath) + 1)) != ERROR_SUCCESS) {
            c_R("Unable to set new PATH. Error: %lu\n", GetLastError());
            c_M("Ensure you have permissions to modify the PATH variable and try again.\n");
            RegCloseKey(hKey);
            return -1;
        }
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
        c_G("Path removed successfully from %s.\n", (mode == MODE_SYSTEM) ? "system" : "user");
    } else {
        c_M("The specified path was not found in the PATH variable.\n");
    }

    RegCloseKey(hKey);
    return 0;
}

// 現在のパス一覧を表示する関数
int ppth(int mode) {
    HKEY hKey;
    LPCTSTR targetKey = (mode == MODE_SYSTEM) ? TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment") : TEXT("Environment");
    HKEY rootKey = (mode == MODE_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    if (RegOpenKeyEx(rootKey, targetKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        c_R("Unable to open registry key. \n");
        return -1;
    }

    char currentPath[32767];
    DWORD pathLen = sizeof(currentPath);
    if (RegQueryValueEx(hKey, TEXT("Path"), NULL, NULL, (LPBYTE)currentPath, &pathLen) != ERROR_SUCCESS) {
        c_R("Unable to read PATH variable. Error: %lu\n", GetLastError());
        RegCloseKey(hKey);
        return -1;
    }

    RegCloseKey(hKey);

    c_G("Current PATH for %s:\n", (mode == MODE_SYSTEM) ? "System" : "User");
    char *token = strtok(currentPath, ";");
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, ";");
    }
    return 0;
}

void path_help() {
    c_B("Usage: path [option] [path]\n");
    c_B("Options:\n");
    c_B("  -a, --add [path]      Add a specified path to the PATH environment variable.\n");
    c_B("      -s, --system      Use this flag with '-a' to add to the system PATH.\n");
    c_B("      -u, --user        Use this flag with '-a' to add to the user PATH. This is the default behavior.\n");
    c_B("  -c, --current         Add the current directory to the PATH (user PATH by default).\n");
    c_B("      -s, --system      Use this flag with '-c' to add to the system PATH.\n");
    c_B("  -d, --delete [path]   Delete a specified path from the PATH environment variable.\n");
    c_B("      -s, --system      Use this flag with '-d' to delete from the system PATH.\n");
    c_B("      -u, --user        Use this flag with '-d' to delete from the user PATH. This is the default behavior.\n");
    c_B("  -p, --print           Print the current PATH. Uses mode flags '-s' or '-u' to select between system or user PATH.\n");
    c_B("  -h, --help            Show this help message and exit.\n");
    c_B("\n");
    c_B("Examples:\n");
    c_B("  hi-c path -a -u ./    Add the current directory to the user PATH\n");
    c_B("  hi-c path -c          Add the current directory to the user PATH (equivalent to above)\n");
    c_B("  hi-c path -c -s       Add the current directory to the system PATH\n");
}


int path(int argc, char *argv[]) {
    int mode = MODE_USER;
    char *target_path = NULL;
    int action = 0; // 0: no action, 1: add, 2: delete, 3: print, 4: add current

    // Parse arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--system") == 0) {
            mode = MODE_SYSTEM;
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) {
            mode = MODE_USER;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--add") == 0) {
            action = 1;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
            action = 2;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print") == 0) {
            action = 3;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--current") == 0) {
            action = 4;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            path_help();
            return 0;
        } else if (target_path == NULL) {
            target_path = argv[i];
        }
    }

    // Execute the appropriate action
    switch (action) {
        case 1: // add
            if (target_path == NULL) {
                c_R("Error: No path specified for add operation.\n");
                path_help();
                return -1;
            }
            return apth(target_path, mode);
        case 2: // delete
            if (target_path == NULL) {
                c_R("Error: No path specified for delete operation.\n");
                path_help();
                return -1;
            }
            return dpth(target_path, mode);
        case 3: // print
            return ppth(mode);
        case 4: // add current
        {
            char current_dir[MAX_PATH];
            if (GetCurrentDirectory(MAX_PATH, current_dir) == 0) {
                c_R("Error: Unable to get current directory.\n");
                return -1;
            }
            return apth(current_dir, mode);
        }
        default:
            c_R("Error: No valid action specified.\n");
            path_help();
            return -1;
    }
}