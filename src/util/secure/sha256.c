// sha256.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../security.h"

#define uchar unsigned char
#define uint  unsigned int

#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

typedef struct {
    uchar data[64];
    uint size;
    uint bits[2];
    uint state[8];
} SHA256_CTX;

uint k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

uint H[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

void SHA256Transform(SHA256_CTX *ctx, uchar data[]){
    uint a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    }

    for (; i < 64; ++i){
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void SHA256Init(SHA256_CTX *ctx){
    ctx->size = 0;
    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
    for (uint i = 0; i < 8; ++i) {
        ctx->state[i] = H[i];
    }
}

void SHA256Update(SHA256_CTX *ctx, uchar data[], uint len){
    for (uint i = 0; i < len; ++i) {
        ctx->data[ctx->size] = data[i];
        ctx->size++;
        if (ctx->size == 64) {
            SHA256Transform(ctx, ctx->data);
            DBL_INT_ADD(ctx->bits[0], ctx->bits[1], 512)
            ctx->size = 0;
        }
    }
}

void SHA256Final(SHA256_CTX *ctx, uchar hash[]) {
    uint i = ctx->size;

    if (ctx->size < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) {
            ctx->data[i++] = 0x00;
        }
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) {
            ctx->data[i++] = 0x00;
        }
        SHA256Transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    DBL_INT_ADD(ctx->bits[0], ctx->bits[1], ctx->size * 8)
    for (uint x = 0; x < 4; ++x) {
        ctx->data[63 - x] = ctx->bits[0] >> (x * 8);
    }
    for (uint x = 0; x < 4; ++x) {
        ctx->data[59 - x] = ctx->bits[1] >> (x * 8);
    }
    SHA256Transform(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        for (uint j = 0; j < 8; ++j) {
            uint x = i + j * 4;
            hash[x] = (ctx->state[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}

char* SHA256F(char *file_path) {
    FILEX fx = {0};
    fx.fileName = file_path;
    if (readFX(&fx) != 0) {
        return NULL;  // ファイル読み込みエラー
    }

    if (fx.fileSize == 0) {
        freeFX(&fx);
        return NULL;
    }

    SHA256_CTX ctx;
    uchar hash[32];
    char* hashStr = (char*)malloc(65);
    if (!hashStr) {
        freeFX(&fx);
        return NULL;
    }
    SHA256Init(&ctx);
    SHA256Update(&ctx, fx.buffer, fx.fileSize);
    SHA256Final(&ctx, hash);

    for (int i = 0; i < 32; i++) {
        sprintf(&hashStr[i * 2], "%02x", hash[i]);
    }
    hashStr[64] = '\0';

    freeFX(&fx);
    return hashStr;
}

char* SHA256(char* data) {
    size_t strLen = strlen(data);
    SHA256_CTX ctx;
    unsigned char hash[32];
    char* hashStr = malloc(65);
    strcpy(hashStr, "");

    SHA256Init(&ctx);
    SHA256Update(&ctx, data, strLen);
    SHA256Final(&ctx, hash);

    char s[3];
    for (int i = 0; i < 32; i++) {
        sprintf(s, "%02x", hash[i]);
        strcat(hashStr, s);
    }

    return hashStr;
}

