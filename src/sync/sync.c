#include "sync.h"

static bool create_directory(const char* dirName) {
    if (CreateDirectoryA(dirName, NULL)) {
        c_M("Directory created: %s\n", dirName);
        return true;
    } else {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_ALREADY_EXISTS) {
            return true;
        } else {
            c_R("Failed to create directory: %s. Error: %lu\n", dirName, dwError);
            return false;
        }
    }
}

int copy_file(const char* src, const char* dst) {
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    int success = 0;

    HANDLE hSrc = CreateFileA(src, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hDst = CreateFileA(dst, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSrc != INVALID_HANDLE_VALUE && hDst != INVALID_HANDLE_VALUE) {
        while (ReadFile(hSrc, buffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
            if (!WriteFile(hDst, buffer, bytesRead, &bytesWritten, NULL) || bytesRead != bytesWritten) {
                c_R("Error writing to file: %s\n", dst);
                break;
            }
        }
        success = 1;
    } else {
        c_R("Failed to open source or destination file. Error: %lu\n", GetLastError());
    }

    CloseHandle(hSrc);
    CloseHandle(hDst);
    return success;
}

void backup(const char* srcPath, const char* backupBasePath) {
    WIN32_FIND_DATA findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char srcFilePath[MAX_PATH];
    char dstFilePath[MAX_PATH];
    char searchPath[MAX_PATH];
    char newDirPath[MAX_PATH];
    sprintf(searchPath, "%s\\*", srcPath);

    hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        c_R("Error finding files in directory: %s\n", srcPath);
        return;
    }

    do {
        if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
            sprintf(srcFilePath, "%s\\%s", srcPath, findData.cFileName);
            sprintf(newDirPath, "%s\\%s", backupBasePath, findData.cFileName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                create_directory(newDirPath);
                backup(srcFilePath, newDirPath);
            } else {
                char *lastSlash = strrchr(newDirPath, '\\');
                if (lastSlash) {
                    *lastSlash = 0;
                    create_directory(newDirPath);  // ディレクトリを確実に作成
                    *lastSlash = '\\';
                }

                if (!copy_file(srcFilePath, newDirPath)) {
                    printf("Failed to copy file: %s to %s\n", srcFilePath, newDirPath);
                }
            }
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

void sync_help() {
    c_B("Usage: sync [options] <source directory>\n");
    c_B("Options:\n");
    c_B("  -b, --backup <dir>          Backup the specified directory to the default backup location.\n");
    c_B("  -c, --cache <dir>           Create a cache of the specified directory.\n");
    c_B("  -h, --help                  Show this help message and exit.\n");
    c_B("\n");
}

int sync(int argc, char **argv) {
    for (int i = 2; i < argc ; i ++ ){
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--backup") == 0) {
            char *src = argv[i + 1];
            if (create_directory(SYNC_DIR)) {
                backup(src, SYNC_DIR);
                cache(SYNC_DIR);
            }
        }
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cache") == 0) {
            char *dir = argv[i + 1];
            if (dir != NULL) {
                cache(dir);
            }
        }
    }
    return 0;
}
