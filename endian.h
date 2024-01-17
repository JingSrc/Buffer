#pragma once

#include <cstdint>

static bool isBigEndian()
{
    union {
        unsigned int i;
        char c[sizeof(int)];
    } u;
    u.i = 1;
    return (u.c[0] != 1);
}

template<typename T> T swap(T v);

template<> inline uint64_t swap<uint64_t>(uint64_t v)
{
    return 0
           | ((v & uint64_t(0x00000000000000ff)) << 56)
           | ((v & uint64_t(0x000000000000ff00)) << 40)
           | ((v & uint64_t(0x0000000000ff0000)) << 24)
           | ((v & uint64_t(0x00000000ff000000)) << 8 )
           | ((v & uint64_t(0x000000ff00000000)) >> 8 )
           | ((v & uint64_t(0x0000ff0000000000)) >> 24)
           | ((v & uint64_t(0x00ff000000000000)) >> 40)
           | ((v & uint64_t(0xff00000000000000)) >> 56);
}

template<> inline uint32_t swap<uint32_t>(uint32_t v)
{
    return 0
           | ((v & 0x000000ff) << 24)
           | ((v & 0x0000ff00) << 8 )
           | ((v & 0x00ff0000) >> 8 )
           | ((v & 0xff000000) >> 24);
}

template<> inline uint16_t swap<uint16_t>(uint16_t v)
{
    return uint16_t(0 | ((v & 0x00ff) << 8) | ((v & 0xff00) >> 8));
}

template<> inline uint8_t swap<uint8_t>(uint8_t v)
{
    return v;
}

template<> inline int64_t swap<int64_t>(int64_t v)
{
    return int64_t(swap<uint64_t>(uint64_t(v)));
}

template<> inline int32_t swap<int32_t>(int32_t v)
{
    return int32_t(swap<uint32_t>(uint32_t(v)));
}

template<> inline int16_t swap<int16_t>(int16_t v)
{
    return int16_t(swap<uint16_t>(uint16_t(v)));
}

template<> inline int8_t swap<int8_t>(int8_t v)
{
    return v;
}

template<typename T> inline T toBigEndian(T v)
{
    return isBigEndian() ? v : swap<T>(v);
}

template<typename T> inline T fromBigEndian(T v)
{
    return isBigEndian() ? v : swap<T>(v);
}
