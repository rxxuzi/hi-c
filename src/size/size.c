#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <inttypes.h>
#include "../util.h"

#define MAX_ITEMS 1000
#define MAX_NAME_LENGTH 20

typedef struct {
    char name[256];
    int64_t size;
    double percentage;
} ItemInfo;

int64_t calc_size(const char *path);
void print_sorted_sizes(const char *path);
int compare_items(const void *a, const void *b);
void print_single_size(const char *path);

void size_help() {
    c_B("Usage: hi-c size [--sort] <path>\n");
    c_B("   --sort: Sort the contents by size (descending order)\n");
    c_B("   path: The path to the file or directory.\n");
}

int size(int argc, char *argv[]) {
    if (argc < 2) {
        size_help();
        return 1;
    }

    bool sort = false;
    const char *path;

    if (argc == 3) {
        path = argv[2];
    } else if (argc == 4 && strcmp(argv[2], "--sort") == 0) {
        sort = true;
        path = argv[3];
    } else {
        size_help();
        return 1;
    }

    if (sort) {
        print_sorted_sizes(path);
    } else {
        print_single_size(path);
    }

    return 0;
}

int64_t calc_size(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        perror("stat");
        return 0;
    }

    int64_t totalSize = 0;

    if (S_ISREG(statbuf.st_mode)) {
        totalSize = (int64_t)statbuf.st_size;
    } else if (S_ISDIR(statbuf.st_mode)) {
        DIR *dir;
        struct dirent *entry;
        char fullPath[1024];

        dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            return 0;
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
            totalSize += calc_size(fullPath);
        }
        closedir(dir);
    }
    return totalSize;
}

int compare_items(const void *a, const void *b) {
    int64_t diff = ((ItemInfo *)b)->size - ((ItemInfo *)a)->size;
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}

void print_sorted_sizes(const char *path) {
    DIR *dir;
    struct dirent *entry;
    char fullPath[1024];
    ItemInfo items[MAX_ITEMS];
    int item_count = 0;
    int64_t total_size = calc_size(path);

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL && item_count < MAX_ITEMS) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        strncpy(items[item_count].name, entry->d_name, sizeof(items[item_count].name) - 1);
        items[item_count].size = calc_size(fullPath);
        items[item_count].percentage = (double)items[item_count].size / total_size * 100;
        item_count++;
    }
    closedir(dir);

    qsort(items, item_count, sizeof(ItemInfo), compare_items);

    c_G("Directory content size breakdown: %s\n\n", path);
    c_G("%-*s %-15s %-10s\n", MAX_NAME_LENGTH, "Name", "Size", "Percentage");
    c_G("%.*s\n", MAX_NAME_LENGTH + 27, "--------------------------------------------");

    for (int i = 0; i < item_count; i++) {
        char *fmt_size = getFileSize64(items[i].size);
        if (fmt_size != NULL) {
            char truncated_name[MAX_NAME_LENGTH + 1];
            strncpy(truncated_name, items[i].name, MAX_NAME_LENGTH);
            truncated_name[MAX_NAME_LENGTH] = '\0';

            if (strlen(items[i].name) > MAX_NAME_LENGTH) {
                truncated_name[MAX_NAME_LENGTH - 1] = '.';
                truncated_name[MAX_NAME_LENGTH - 2] = '.';
                truncated_name[MAX_NAME_LENGTH - 3] = '.';
            }

            c_G("%-*s %-15s %6.2f%%\n", MAX_NAME_LENGTH, truncated_name, fmt_size, items[i].percentage);
            free(fmt_size);
        }
    }

    char *total_fmt_size = getFileSize64(total_size);
    if (total_fmt_size != NULL) {
        c_G("\n%-*s %-15s %6.2f%%\n", MAX_NAME_LENGTH, "Total", total_fmt_size, 100.0);
        free(total_fmt_size);
    }
}

void print_single_size(const char *path) {
    char *fmt_size = getFileSize64(calc_size(path));
    if (fmt_size == NULL) {
        c_R("Failed to get File size.\n");
    } else {
        c_G("Size : %s\n", fmt_size);
        free(fmt_size);
    }
}