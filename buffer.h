#pragma once

#include <string>
#include <fstream>

class BufferPrivate;

class Buffer
{
public:
    explicit Buffer(int size = -1);
    explicit Buffer(const char* data, int size = -1);
    explicit Buffer(const std::string& data);
    ~Buffer();

    Buffer(const Buffer& other);
    Buffer(Buffer&& other);

    Buffer& operator=(const Buffer& other);
    Buffer& operator=(Buffer&& other);

    char& operator[](int i);
    char operator[](int i) const;

    bool operator==(const Buffer& other) const;
    bool operator!=(const Buffer& other) const;

    bool isEmpty() const;
    int size() const;

    const char* data() const;
    char* data();

    void swap(Buffer& other);

    Buffer& resize(int size);
    Buffer& truncate(int size);

    Buffer& append(const Buffer& buffer);
    Buffer& append(char ch);
    Buffer& append(const char* data, int size = -1);
    Buffer& append(const std::string& data);

    Buffer& insert(int pos, const Buffer& data);
    Buffer& insert(int pos, char ch);
    Buffer& insert(int pos, const char* data, int size = -1);
    Buffer& insert(int pos, const std::string& data);

    void remove(int pos, int len);
    void removeAt(int pos);

    void clear();

    Buffer mid(int pos, int len = -1) const;

    std::string toString(int len = -1) const;

    std::string toHex() const;
    static Buffer fromHex(const std::string& hex);

    std::string toBase64() const;
    static Buffer fromBase64(const std::string& base64);

private:
    BufferPrivate* m_ptr;
};

class BufferWriter
{
public:
    explicit BufferWriter(Buffer& buffer);
    ~BufferWriter();

    BufferWriter& write(const char* data, int len);

    BufferWriter& operator<<(uint8_t value);
    BufferWriter& operator<<(uint16_t value);
    BufferWriter& operator<<(uint32_t value);
    BufferWriter& operator<<(uint64_t value);

    BufferWriter& operator<<(int8_t value);
    BufferWriter& operator<<(int16_t value);
    BufferWriter& operator<<(int32_t value);
    BufferWriter& operator<<(int64_t value);

    BufferWriter& operator<<(const char* value);
    BufferWriter& operator<<(const std::string& value);

    BufferWriter& operator<<(const Buffer& value);

private:
    Buffer& m_buffer;
};

class BufferReader
{
public:
    explicit BufferReader(const Buffer& buffer);
    ~BufferReader();

    int read(char* buffer, int len);

    int seek(int position);
    int position() const;

    bool atEnd() const;

    BufferReader& operator>>(uint8_t &value);
    BufferReader& operator>>(uint16_t &value);
    BufferReader& operator>>(uint32_t &value);
    BufferReader& operator>>(uint64_t &value);

    BufferReader& operator>>(int8_t &value);
    BufferReader& operator>>(int16_t &value);
    BufferReader& operator>>(int32_t &value);
    BufferReader& operator>>(int64_t &value);

    BufferReader& operator>>(std::string& value);

    BufferReader& operator>>(Buffer& buffer);

private:
    const Buffer& m_buffer;
    int m_position;
};

std::ofstream& operator<<(std::ofstream& stream, const crypto::Buffer& buffer);
std::ifstream& operator>>(std::ifstream& stream, crypto::Buffer& buffer);
