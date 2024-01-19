#pragma once

class Buffer;

class sm4
{
public:
    sm4() = delete;
    ~sm4() = delete;

    static bool encrypt(const char* data, int len, const Buffer& key, Buffer& output);
    static bool encrypt(const Buffer& data, const Buffer& key, Buffer& output);

    static bool decrypt(const char* data, int len, const Buffer& key, Buffer& output);
    static bool decrypt(const Buffer& data, const Buffer& key, Buffer& output);
};
