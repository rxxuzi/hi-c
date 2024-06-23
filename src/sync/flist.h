#ifndef HI_C_FLIST_H
#define HI_C_FLIST_H

typedef struct {
    char *src;         // パスを保存
    bool is_dir;       // パスがディレクトリかファイルか
    bool recursive;    // フォルダ内を再帰的に探索するか
    char **path;       // ファイルパスを格納する配列
    char **hash;       // ファイルハッシュを格納する配列
    size_t size;       // 格納されたファイルパスの数
} FLIST;

FLIST *newFLIST(const char *src, bool recursive);
void initFLIST(FLIST *flist);
void showFLIST(FLIST *flist);
void freeFLIST(FLIST *flist);
void calcHash(FLIST *flist);


#endif //HI_C_FLIST_H
