#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>
#include "stat.h"
#include "../util.h"

#define ok 0
#define error (-1)
#define ull unsigned long long
#define REFRESH 200

static int cpu_flag = 0;
static int bar_length = 20;

/*
 * style of :
 * 35% [#######................]
 */
char* bar(ull total, ull used) {
    int length = bar_length;
    ull percent = (used * 100) / total;
    int filled = (int)(length * percent) / 100; // Correctly calculate number of filled positions
    int rest = length - filled;
    char* bar = (char*)malloc((length + 10) * sizeof(char)); // Ensure enough space for the bar and percentage
    if (!bar) return NULL; // Check for malloc failure

    sprintf(bar, "%02llu %% [", percent);
    for (int i = 0; i < filled; i++) {
        strcat(bar, "#");
    }
    for (int i = 0; i < rest; i++) {
        strcat(bar, ".");
    }
    strcat(bar, "]");

    return bar;
}

// Memory status function
int ram() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) {
        c_R("Failed to get memory status.\n");
        return error;
    }

    char *st_bar = bar(statex.ullTotalPhys, statex.ullTotalPhys - statex.ullAvailPhys);
    c_G("Mem  usage: %s\n", st_bar);
    free(st_bar);
    return ok;
}

// CPU usage function
int cpu() {
    static FILETIME preIdleTime = {0}, preKernelTime = {0}, preUserTime = {0};
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        c_R("Failed to get system times.\n");
        return error;
    }

    if (preIdleTime.dwLowDateTime == 0 && preIdleTime.dwHighDateTime == 0) {
        preIdleTime = idleTime;
        preKernelTime = kernelTime;
        preUserTime = userTime;
        return ok;  // 最初の呼び出しでは測定不可能なため、次の呼び出しまで待つ
    }

    ULONGLONG idle = *((ULONGLONG*)&idleTime) - *((ULONGLONG*)&preIdleTime);
    ULONGLONG kernel = *((ULONGLONG*)&kernelTime) - *((ULONGLONG*)&preKernelTime);
    ULONGLONG user = *((ULONGLONG*)&userTime) - *((ULONGLONG*)&preUserTime);

    ULONGLONG total = kernel + user;

    char* st_bar = bar(total, (total - idle));
    c_G("CPU  usage: %s\n", st_bar);

    preIdleTime = idleTime;
    preKernelTime = kernelTime;
    preUserTime = userTime;
    free(st_bar);  // mallocで確保したメモリを解放
    return ok;
}


// Disk usage function
int disk() {
    ULARGE_INTEGER freeBytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (!GetDiskFreeSpaceEx(NULL, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        c_R("Failed to get disk space information.\n");
        return error;
    }
    
    char *st_bar = bar(totalNumberOfBytes.QuadPart, totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart);
    c_G("Disk usage: %s\n", st_bar);
    free(st_bar);
    return ok;
}

// Heap usage function
int heap() {
    HANDLE hHeap = GetProcessHeap();
    HEAP_SUMMARY summary;
    summary.cb = sizeof(HEAP_SUMMARY);

    if (!HeapSummary(hHeap, 0, &summary)) {
        c_R("Failed to get heap summary. Error: %lu\n", GetLastError());
        return error;
    }
    
    char *st_bar = bar(summary.cbCommitted, summary.cbAllocated);
    c_G("Heap usage: %s\n", st_bar);
    free(st_bar);
    return ok;
}

int echo(){
    if (ram() == error || cpu() == error || disk() == error || heap() == error) {
        c_R("Failed to retrieve one or more system stats.\n");
        return error;
    }
    Sleep(500);
    if (cpu_flag == 0) {
        cpu_flag = 1;
        if (cpu() == error) {
            c_R("Failed to retrieve CPU stats.\n");
            return error;
        }
    }

    return ok;
}

void stat_help() {
    c_B("Usage: stat [options]\n");
    c_B("Options:\n");
    c_B("  --listen, -l        Listen mode: continuously displays system stats every %d ms.\n", REFRESH);
    c_B("                      Press Ctrl+C to exit this mode.\n");
    c_B("  --all, -a           Display all system stats once.\n");
    c_B("  --ram               Display current RAM usage immediately.\n");
    c_B("  --cpu               Display CPU usage. Note: CPU usage is measured over a short interval.\n");
    c_B("  --disk              Display current disk usage immediately.\n");
    c_B("  --heap              Display current heap usage immediately.\n");
    c_B("  --len=<n>           Set the length of the status bar for some displays. Valid range: 5-100.\n");
    c_B("                      Note: This option must be used in conjunction with other options.\n");
    c_B("\n");
    c_B("Default behavior:\n");
    c_B("  If no arguments are provided, the program starts in listen mode with a default status bar length of 20.\n");
    c_B("\n");
}

int stat_listen() {
    initCursorPosition();  // カーソルの初期位置を保存
    resetCursorPosition();
    while (1) {
        resetCursorPosition();  // カーソル位置を初期位置に戻す
        c_M("Press Ctrl+C to stop the monitoring.\n");
        if (echo() == error) {
            break;
        }
        Sleep(REFRESH);
    }
    return error;
}

int stats(int argc, char **argv) {
    if (argc == 2) {
        stat_listen();
    }
    // search for "--len=n"
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--len=", 6) == 0) {
            bar_length = atoi(argv[i] + 6);
            if (bar_length < 5 || bar_length > 100) {
                c_R("Invalid bar length %d. Using default value of 20.\n", bar_length);
                bar_length = 20;
            }
            break;
        }
    }

    initCursorPosition();
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--listen") == 0) {
            stat_listen();
        }
        else if (strcmp(argv[i], "--all") == 0 || strcmp(argv[i], "-a") == 0) {
            return echo();
        }
        else if (strcmp(argv[i], "--ram") == 0) {
            return ram();
        }
        else if (strcmp(argv[i], "--cpu") == 0) {
            cpu();
            Sleep(250);
            return cpu();
        }
        else if (strcmp(argv[i], "--disk") == 0) {
            return disk();
        }
        else if (strcmp(argv[i], "--heap") == 0) {
            return heap();
        }
    }
    c_R("Invalid option.\n");
    stat_help();
    return error;
}