// flist.c
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "flist.h"
#include "../util/security.h"

void AddPath(FLIST *flist, const char *path) {
    flist->path = realloc(flist->path, (flist->size + 1) * sizeof(char*));
    flist->path[flist->size] = _strdup(path);
    flist->size++;
}

void ListFiles(FLIST *flist, const char *directory) {
    WIN32_FIND_DATA data;
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", directory);  // ディレクトリパスの構築
    HANDLE hFind = FindFirstFile(searchPath, &data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {
                char fullPath[MAX_PATH];
                snprintf(fullPath, sizeof(fullPath), "%s\\%s", directory, data.cFileName);

                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && flist->recursive) {
                    ListFiles(flist, fullPath);  // 再帰的にディレクトリを探索
                } else {
                    AddPath(flist, fullPath);  // ファイルパスをFLISTに追加
                }
            }
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
}

void initFLIST(FLIST *flist) {
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(flist->src, &data);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Unable to find the file or directory %s\n", flist->src);
        return;
    }

    do {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // srcがディレクトリの場合、ディレクトリ内容を探索する
            if (flist->recursive) {
                FindClose(hFind);  // このハンドルを閉じる
                ListFiles(flist, flist->src);  // ディレクトリ内を再帰的に探索
                return;
            }
        } else {
            // srcがファイルの場合、そのパスを追加する
            AddPath(flist, flist->src);
            FindClose(hFind);  // ループを抜ける前にハンドルを閉じる
            return;
        }
    } while (FindNextFile(hFind, &data));
    FindClose(hFind);
}


FLIST *newFLIST(const char *src, bool recursive) {
    FLIST *flist = malloc(sizeof(FLIST));
    flist->src = _strdup(src);
    flist->is_dir = false;
    flist->recursive = recursive;
    flist->path = NULL;
    flist->hash = NULL;
    flist->size = 0;
    return flist;
}

void freeFLIST(FLIST *flist) {
    if (flist == NULL) return;  // flist自体がNULLであれば何もしない

    free(flist->src);
    flist->src = NULL;

    if (flist->path != NULL) {
        for (size_t i = 0; i < flist->size; i++) {
            free(flist->path[i]);
            flist->path[i] = NULL;  // 解放後はNULLを代入
        }
        free(flist->path);
        flist->path = NULL;
    }

    if (flist->hash != NULL) {
        for (size_t i = 0; i < flist->size; i++) {
            free(flist->hash[i]);
            flist->hash[i] = NULL;  // 解放後はNULLを代入
        }
        free(flist->hash);
        flist->hash = NULL;
    }

    free(flist);  // 最後にflist自体を解放
    flist = NULL;  // ローカルのポインタ変数にNULLを代入しても呼び出し元には影響しないが、慣習として
}


void showFLIST(FLIST *flist) {
    printf("SRC : %s\n", flist->src);
    printf("SIZE : %zu\n", flist->size);
    printf("PATHS: <hash:path>\n");
    for (size_t i = 0; i < flist->size; i++) {
        printf("%s : %s\n",flist->hash[i], flist->path[i]);
    }
}

void calcHash(FLIST *flist) {
    if (flist->size == 0) return;  // ハッシュを計算するパスがなければ何もしない

    flist->hash = malloc(flist->size * sizeof(char*));
    if (flist->hash == NULL) {
        fprintf(stderr, "memory allocation failure\n");
        return;  // メモリ確保に失敗した場合、エラーメッセージを出力してリターン
    }

    for (size_t i = 0; i < flist->size; i++) {
        flist->hash[i] = SHA256F(flist->path[i]);
        if (flist->hash[i] == NULL) {
            fprintf(stderr, "hash calculation failure: %s\n", flist->path[i]);
        }
    }
}

int writeFLIST(FLIST *fl, const char *file_name) {
    FILE *fp = fopen(file_name, "w");

    if (fp == NULL) {
        c_R("Error opening cache file for writing\n");
        return EXIT_FAILURE;
    }
    if (fl == NULL) {
        c_R("FLIST is NULL\n");
        return EXIT_FAILURE;
    }

    // write header
    fprintf(fp, "#SRC:%s\n",fl->src);
    fprintf(fp, "#LEN:%zu\n", fl->size);

    for (int i = 0; i < fl->size; i++) {
        fprintf(fp, "%s : %s\n", fl->hash[i], fl->path[i]);
    }
    fclose(fp);
    return EXIT_SUCCESS;
}

// FLIST 構造体を初期化する関数
FLIST *readFlist(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    FLIST *flist = malloc(sizeof(FLIST));
    if (!flist) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    char line[512]; // 1行の最大長を仮定

    // ソースディレクトリを読み込む
    if (fgets(line, sizeof(line), file) == NULL || strncmp(line, "#SRC:", 5) != 0) {
        fprintf(stderr, "Source directory not found.\n");
        free(flist);
        fclose(file);
        return NULL;
    }
    flist->src = strdup(line + 5);
    flist->src[strcspn(flist->src, "\n")] = 0; // 改行を削除

    // ファイルのサイズ（行数）を読み込む
    if (fgets(line, sizeof(line), file) == NULL || strncmp(line, "#LEN:", 5) != 0) {
        fprintf(stderr, "The length of the file is not specified.\n");
        free(flist->src);
        free(flist);
        fclose(file);
        return NULL;
    }
    flist->size = atoi(line + 5);
    flist->is_dir = true;
    flist->recursive = true;
    flist->path = malloc(flist->size * sizeof(char *));
    flist->hash = malloc(flist->size * sizeof(char *));

    // ハッシュとパスを読み込む
    for (size_t i = 0; i < flist->size && fgets(line, sizeof(line), file); i++) {
        char *hash = strtok(line, " ");
        char *path = strtok(NULL, "\n");
        if (hash && path) {
            flist->hash[i] = strdup(hash);
            // パスの前の余計なコロンとスペースを除去
            if (*path == ':') path++; // コロンをスキップ
            while (*path == ' ') path++; // 余計なスペースをスキップ
            flist->path[i] = strdup(path);
        } else {
            fprintf(stderr, "A formatting error occurred.\n");
            // 既に確保されたメモリのクリーンアップ
            for (size_t j = 0; j < i; j++) {
                free(flist->path[j]);
                free(flist->hash[j]);
            }
            free(flist->path);
            free(flist->hash);
            free(flist->src);
            free(flist);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    return flist;
}