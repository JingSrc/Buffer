#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "sm3.h"
#include "buffer.h"

namespace
{

// 32-bit integer manipulation macros (big endian)
#define SM3_GET_ULONG_BE(n, b, i) {       \
(n) = ((uint32_t)(b)[(i)    ] << 24)  \
      | ((uint32_t)(b)[(i) + 1] << 16)  \
      | ((uint32_t)(b)[(i) + 2] <<  8)  \
      | ((uint32_t)(b)[(i) + 3]      ); \
}

#define SM3_PUT_ULONG_BE(n, b, i) {       \
(b)[(i)    ] = (uint8_t)((n) >> 24);  \
    (b)[(i) + 1] = (uint8_t)((n) >> 16);  \
    (b)[(i) + 2] = (uint8_t)((n) >>  8);  \
    (b)[(i) + 3] = (uint8_t)((n)      );  \
}

// SM3 context structure
struct sm3_context
{
    uint32_t total[2];   // number of bytes processed
    uint32_t state[8];   // intermediate digest state
    uint8_t  buffer[64]; // data block being processed

    uint8_t ipad[64];   // HMAC: inner padding
    uint8_t opad[64];   // HMAC: outer padding
};

// SM3 context setup
static void sm3_init(sm3_context* ctx)
{
    static const uint32_t state[8] = {
        0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
        0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
    };

    memset(ctx->total, 0, sizeof(ctx->total));
    memcpy(ctx->state, state, sizeof(ctx->state));
}

static void sm3_process(sm3_context* ctx, const uint8_t data[64])
{
    uint32_t SS1, SS2, TT1, TT2, W[68], W1[64];
    uint32_t A, B, C, D, E, F, G, H;
    uint32_t T[64];
    uint32_t Temp1, Temp2, Temp3, Temp4, Temp5;
    int j;

    for (j = 0; j < 16; ++j)
        T[j] = 0x79CC4519;
    for (j = 16; j < 64; ++j)
        T[j] = 0x7A879D8A;

    SM3_GET_ULONG_BE(W[0], data, 0);
    SM3_GET_ULONG_BE(W[1], data, 4);
    SM3_GET_ULONG_BE(W[2], data, 8);
    SM3_GET_ULONG_BE(W[3], data, 12);
    SM3_GET_ULONG_BE(W[4], data, 16);
    SM3_GET_ULONG_BE(W[5], data, 20);
    SM3_GET_ULONG_BE(W[6], data, 24);
    SM3_GET_ULONG_BE(W[7], data, 28);
    SM3_GET_ULONG_BE(W[8], data, 32);
    SM3_GET_ULONG_BE(W[9], data, 36);
    SM3_GET_ULONG_BE(W[10], data, 40);
    SM3_GET_ULONG_BE(W[11], data, 44);
    SM3_GET_ULONG_BE(W[12], data, 48);
    SM3_GET_ULONG_BE(W[13], data, 52);
    SM3_GET_ULONG_BE(W[14], data, 56);
    SM3_GET_ULONG_BE(W[15], data, 60);

#define FF0(x, y, z) ((x) ^ (y) ^ (z))
#define FF1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))

#define GG0(x, y, z) ((x) ^ (y) ^ (z))
#define GG1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))

#define SHL(x, n) (((x) & 0xFFFFFFFF) << n)
#define ROTL(x, n) (SHL((x), n) | ((x) >> (32 - n)))

#define P0(x) ((x) ^ ROTL((x), 9) ^ ROTL((x), 17))
#define P1(x) ((x) ^ ROTL((x), 15) ^ ROTL((x), 23))

    for (j = 16; j < 68; ++j) {
        Temp1 = W[j - 16] ^ W[j - 9];
        Temp2 = ROTL(W[j - 3], 15);
        Temp3 = Temp1 ^ Temp2;
        Temp4 = P1(Temp3);
        Temp5 = ROTL(W[j - 13], 7) ^ W[j - 6];
        W[j] = Temp4 ^ Temp5;
    }

    for (j = 0; j < 64; ++j) {
        W1[j] = W[j] ^ W[j + 4];
    }

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];

    for (j = 0; j < 16; ++j) {
        SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)), 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = FF0(A, B, C) + D + SS2 + W1[j];
        TT2 = GG0(E, F, G) + H + SS1 + W[j];
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
    }

    for (j = 16; j < 64; ++j) {
        SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)), 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = FF1(A, B, C) + D + SS2 + W1[j];
        TT2 = GG1(E, F, G) + H + SS1 + W[j];
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
    }

    ctx->state[0] ^= A;
    ctx->state[1] ^= B;
    ctx->state[2] ^= C;
    ctx->state[3] ^= D;
    ctx->state[4] ^= E;
    ctx->state[5] ^= F;
    ctx->state[6] ^= G;
    ctx->state[7] ^= H;
}

static void sm3_update(sm3_context* ctx, const uint8_t* input, int ilen)
{
    int      fill;
    uint32_t left;

    if (ilen <= 0)
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (uint32_t)ilen)
        ctx->total[1]++;

    if (left && ilen >= fill) {
        memcpy(ctx->buffer + left, input, fill);
        sm3_process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        sm3_process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0) {
        memcpy(ctx->buffer + left, input, ilen);
    }
}

static void sm3_finish(sm3_context* ctx, uint8_t output[32])
{
    static const uint8_t padding[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    uint32_t last;
    uint32_t padn;
    uint32_t high;
    uint32_t low;
    uint8_t  msglen[8];

    high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    low = (ctx->total[0] << 3);

    SM3_PUT_ULONG_BE(high, msglen, 0);
    SM3_PUT_ULONG_BE(low, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    sm3_update(ctx, (uint8_t*)padding, padn);
    sm3_update(ctx, msglen, 8);

    SM3_PUT_ULONG_BE(ctx->state[0], output, 0);
    SM3_PUT_ULONG_BE(ctx->state[1], output, 4);
    SM3_PUT_ULONG_BE(ctx->state[2], output, 8);
    SM3_PUT_ULONG_BE(ctx->state[3], output, 12);
    SM3_PUT_ULONG_BE(ctx->state[4], output, 16);
    SM3_PUT_ULONG_BE(ctx->state[5], output, 20);
    SM3_PUT_ULONG_BE(ctx->state[6], output, 24);
    SM3_PUT_ULONG_BE(ctx->state[7], output, 28);
}

}

static constexpr int size = 32;

Buffer sm3::encode(const Buffer& data)
{
    if (data.isEmpty()) {
        return Buffer{};
    }

    char buf[32];
    memset(buf, 0, sizeof(buf));

    sm3_context ctx;
    sm3_init(&ctx);
    sm3_update(&ctx, (const uint8_t*)data.data(), data.size());
    sm3_finish(&ctx, (uint8_t*)buf);

    return Buffer{ buf, sizeof(buf) };
}

Buffer sm3::sum(const std::filesystem::path& filePath)
{
    // std::locale::global(std::locale(""));

    auto ifs = fopen(filePath.string().c_str(), "rb");
    if (!ifs) {
        return Buffer{};
    }

    Buffer buffer{ 32 };
    char buf[8192];
    size_t n = 0;

    sm3_context ctx;
    sm3_init(&ctx);
    while ((n = fread(buf, 1, sizeof(buf), ifs)) > 0) {
        sm3_update(&ctx, (uint8_t*)buf, static_cast<int>(n));
    }
    sm3_finish(&ctx, (uint8_t*)buffer.data());
    fclose(ifs);

    return buffer;
}
