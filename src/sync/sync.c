#include "sync.h"
#include "flist.h"
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
    if (hSrc == INVALID_HANDLE_VALUE) {
        c_R("Failed to open source file: %s. Error: %lu\n", src, GetLastError());
        return 0;
    }

    HANDLE hDst = CreateFileA(dst, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDst == INVALID_HANDLE_VALUE) {
        c_R("Failed to create destination file: %s. Error: %lu\n", dst, GetLastError());
        CloseHandle(hSrc);
        return 0;
    }

    while (ReadFile(hSrc, buffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
        if (!WriteFile(hDst, buffer, bytesRead, &bytesWritten, NULL) || bytesRead != bytesWritten) {
            c_R("Error writing to file: %s. Error: %lu\n", dst, GetLastError());
            success = 0;
            break;
        }
        success = 1;
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
    c_B("  -d, --diff [src] [dst]      Show differences between two directories or between cache and a directory.\n");
    c_B("                             - If both src and dst are provided, compare these two directories.\n");
    c_B("                             - If only dst is provided, compare it with the cached directory.\n");
    c_B("                             - If no arguments are provided, compare the cached directory with the default backup location.\n");
    c_B("  -r, --restore [dir]         Restore files from the backup. If [dir] is specified,\n");
    c_B("                              restore to that directory. Otherwise, restore to the original location.\n");
    c_B("  -l, --list                  List contents of the cached directory.\n");
    c_B("  -x, --cleanup               Remove the backup directory and cache file.\n");
    c_B("  -h, --help                  Show this help message and exit.\n");
    c_B("\n");
    c_B("Directory Information:\n");
    c_B("  Backup Directory:           " SYNC_DIR "\n");
    c_B("  Cache File:                 " SYNC_CACHE "\n");
    c_B("\n");
}

int sync(int argc, char **argv) {
    for (int i = 2; i < argc ; i ++ ){
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--backup") == 0) {
            char *src = argv[i + 1];
            if (create_directory(SYNC_DIR)) {
                backup(src, SYNC_DIR);
                cache(src);
            }
        }
        // cacheをとる。 -c <path>
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cache") == 0) {
            char *dir = argv[i + 1];
            if (dir != NULL) {
                return cache(dir);
            }
            return 1;
        }

        if (strcmp(argv[i],"-d") == 0 || strcmp(argv[i], "--diff") == 0) {
            c_M("[+] for Add: File exists in source but not in destination.\n");
            c_M("[-] for Delete: File exists in destination but not in source.\n");
            c_M("[!] for Modify: File exists in both but is different.\n");
            if (i + 3 == argc) {
                // -d <src> <dct>
                char *src = argv[i + 1];
                char *dct = argv[i + 2];
                if (src == NULL || dct == NULL) {
                    c_R("Invalid arguments for -d option.\n");
                    return 1;
                }
                diff(src, dct);
            } else if (i + 2 == argc){
                // -d cache内にあるsrcと指定のディレクトリ
                FLIST *src = readFlist(SYNC_CACHE);
                diff(src->src, argv[i + 1]);
            } else {
                // -d cacheとデフォルトディレクトリの差分
                FLIST *tmp = readFlist(SYNC_CACHE);
                diff(tmp->src, SYNC_DIR);
            }
            return 0;
        }

        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            FLIST *tmp = readFlist(SYNC_CACHE);
            showFLIST(tmp);
            freeFLIST(tmp);
            return 0;
        }

        if (strcmp(argv[2], "-r") == 0 || strcmp(argv[2], "--restore") == 0) {
            if (argc > 3) {
                return restore(argv[3]);
            } else {
                // キャッシュファイルから元のディレクトリを取得して使用
                FLIST *cacheList = readFlist(SYNC_CACHE);
                if (!cacheList) {
                    c_R("Failed to read cache file.\n");
                    return EXIT_FAILURE;
                }
                int result = restore(cacheList->src);
                freeFLIST(cacheList);
                return result;
            }
        }

        if (strcmp(argv[2], "-x") == 0 || strcmp(argv[2], "--cleanup") == 0) {
            return cleanup();
        }

        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            sync_help();
            return 0;
        }
    }
    return 0;
}
