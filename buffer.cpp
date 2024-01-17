#include <cstring>

#include "buffer.h"
#include "endian.h"

class Base64
{
public:
    static std::string encode(const Buffer& buffer);
    static Buffer decode(const std::string& base64);

private:
    static inline bool isBase64(unsigned char ch) {
        return (std::isalnum(ch) || (ch == '+') || (ch == '/'));
    }

private:
    static const std::string Chars;
};

const std::string Base64::Chars{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };

class Hex
{
public:
    static std::string encode(const Buffer& buffer);
    static Buffer decode(const std::string& hex);

private:
    static inline bool isHex(char ch) {
        return ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'));
    }

    static inline char hexToUpper(char ch) {
        switch (ch) {
        case 'a': return 'A';
        case 'b': return 'B';
        case 'c': return 'C';
        case 'd': return 'D';
        case 'e': return 'E';
        case 'f': return 'F';
        default: break;
        }

        return ch;
    }

private:
    static const std::string Chars;
};

const std::string Hex::Chars{ "0123456789ABCDEF" };

class BufferPrivate
{
public:
    explicit BufferPrivate(int size = -1);
    explicit BufferPrivate(const char* data, int size = -1);
    ~BufferPrivate();

    bool isEmpty() const { return (m_size == 0); }
    int size() const { return m_size; }
    int capacity() const { return m_capacity; }

    const char* data() const { return m_data; }
    char* data() { return m_data; }

    void reserve(int size);
    void resize(int size);
    void truncate(int size);

    void insert(int pos, const char* data, int size);
    void remove(int pos, int len);
    void clear();

    BufferPrivate& operator=(const BufferPrivate& other);
    BufferPrivate& operator=(BufferPrivate&& other);

private:
    int   m_size;
    int   m_capacity;
    char* m_data;
};

BufferPrivate::BufferPrivate(int size)
    : m_size{ 0 }
    , m_capacity{ 0 }
    , m_data{ nullptr }
{
    resize(size);
}

BufferPrivate::BufferPrivate(const char* data, int size)
    : m_size{ 0 }
    , m_capacity{ 0 }
    , m_data{ nullptr }
{
    insert(0, data, size);
}

BufferPrivate::~BufferPrivate()
{
    clear();
}

void BufferPrivate::reserve(int size)
{
    if (size <= m_capacity) {
        return;
    }

    auto tmp = reinterpret_cast<char*>(std::malloc(size));
    std::memset(tmp, 0, size);
    if (m_size > 0) {
        std::memcpy(tmp, m_data, m_size);
        std::free(m_data);
    }

    m_data = tmp;
    m_capacity = size;
}

void BufferPrivate::resize(int size)
{
    reserve(size);
    if (size >= 0) {
        m_size = size;
    }
}

void BufferPrivate::truncate(int size)
{
    if (m_size <= size) {
        return;
    }
    if (size == 0) {
        clear();
        return;
    }

    auto tmp = reinterpret_cast<char*>(std::malloc(size));
    if (m_size > 0) {
        std::memcpy(tmp, m_data, size);
        std::free(m_data);
    }

    m_size = size;
    m_capacity = size;
    m_data = tmp;
}

void BufferPrivate::insert(int pos, const char* data, int size)
{
    if (!data) {
        return;
    }

    auto len = size;
    if (len < 0) {
        len = static_cast<int>(std::strlen(data)) + 1;
    }

    auto left = m_capacity - m_size;
    if (len > left) {
        left += (((len + 64) / 64 + 1) * 64);
        reserve(m_capacity + left);
    }
    if (!m_data) {
        return;
    }

    if (pos >= m_size) {
        std::memcpy(m_data + m_size, data, len);
    }
    else {
        if (m_size > 0) {
            std::memmove(m_data + pos + len, m_data + pos, m_size - pos);
        }
        std::memcpy(m_data + pos, data, len);
    }
    m_size += len;
}

void BufferPrivate::remove(int pos, int len)
{
    std::memmove(m_data + pos, m_data + pos + len, m_size - pos - len);
    m_size -= len;
}

void BufferPrivate::clear()
{
    m_size = 0;
    m_capacity = 0;

    if (m_data) {
        std::free(m_data);
        m_data = nullptr;
    }
}

BufferPrivate& BufferPrivate::operator=(const BufferPrivate& other)
{
    if (this == &other) {
        return *this;
    }

    if (m_capacity != other.m_capacity) {
        clear();
        if (other.m_size > 0) {
            resize(other.m_size);
        }
    }

    if (other.m_size > 0) {
        std::memcpy(m_data, other.m_data, other.m_size);
    }
    return *this;
}

BufferPrivate& BufferPrivate::operator=(BufferPrivate&& other)
{
    if (this == &other) {
        return *this;
    }

    clear();

    m_size = other.m_size;
    m_capacity = other.m_capacity;
    m_data = other.m_data;

    other.m_size = 0;
    other.m_capacity = 0;
    other.m_data = nullptr;

    return *this;
}

Buffer::Buffer(int size)
    : m_ptr{ new BufferPrivate{size} }
{

}

Buffer::Buffer(const char* data, int size)
    : m_ptr{ new BufferPrivate{data, size} }
{

}

Buffer::Buffer(const std::string& data)
    : Buffer{ data.c_str(), static_cast<int>(data.size()) }
{

}

Buffer::~Buffer()
{
    delete m_ptr;
}

Buffer::Buffer(const Buffer& other)
    : m_ptr{ new BufferPrivate }
{
    *m_ptr = *other.m_ptr;
}

Buffer::Buffer(Buffer&& other)
    : m_ptr{ new BufferPrivate }
{
    *m_ptr = std::move(*other.m_ptr);
}

bool Buffer::isEmpty() const
{
    return m_ptr->isEmpty();
}

int Buffer::size() const
{
    return m_ptr->size();
}

const char* Buffer::data() const
{
    return m_ptr->data();
}

char* Buffer::data()
{
    return m_ptr->data();
}

void Buffer::swap(Buffer& other)
{
    std::swap(m_ptr, other.m_ptr);
}

char& Buffer::operator[](int i)
{
    return m_ptr->data()[i];
}

bool Buffer::operator==(const Buffer& other) const
{
    if (m_ptr->size() != other.size()) {
        return false;
    }

    return (std::memcmp(data(), other.data(), m_ptr->size()) == 0);
}

bool Buffer::operator!=(const Buffer& other) const
{
    return !(*this == other);
}

char Buffer::operator[](int i) const
{
    return m_ptr->data()[i];
}

Buffer& Buffer::operator=(const Buffer& other)
{
    *m_ptr = *other.m_ptr;
    return *this;
}

Buffer& Buffer::operator=(Buffer&& other)
{
    *m_ptr = std::move(*other.m_ptr);
    return *this;
}

Buffer& Buffer::resize(int size)
{
    m_ptr->resize(size);
    return *this;
}

Buffer& Buffer::truncate(int size)
{
    m_ptr->truncate(size);
    return *this;
}

Buffer& Buffer::append(const Buffer& buffer)
{
    append(buffer.data(), buffer.size());
    return *this;
}

Buffer& Buffer::append(char ch)
{
    append(&ch, 1);
    return *this;
}

Buffer& Buffer::append(const char* data, int size)
{
    insert(m_ptr->size(), data, size);
    return *this;
}

Buffer& Buffer::append(const std::string& data)
{
    append(data.c_str(), static_cast<int>(data.size()));
    append('\0');
    return *this;
}

Buffer& Buffer::insert(int pos, const Buffer& data)
{
    insert(pos, data.data(), data.size());
    return *this;
}

Buffer& Buffer::insert(int pos, char ch)
{
    insert(pos, &ch, 1);
    return *this;
}

Buffer& Buffer::insert(int pos, const char* data, int size)
{
    m_ptr->insert(pos, data, size);
    return *this;
}

Buffer& Buffer::insert(int pos, const std::string& data)
{
    insert(pos, data.c_str(), static_cast<int>(data.size()));
    return *this;
}

void Buffer::remove(int pos, int len)
{
    m_ptr->remove(pos, len);
}

void Buffer::removeAt(int pos)
{
    remove(pos, 1);
}

void Buffer::clear()
{
    m_ptr->clear();
}

Buffer Buffer::mid(int pos, int len) const
{
    if (len < 0) {
        len = m_ptr->size() - pos;
    }

    return Buffer{ m_ptr->data() + pos, len };
}

std::string Buffer::toString(int len) const
{
    auto size = static_cast<size_t>(m_ptr->size());
    if (len > 0 && len < m_ptr->size()) {
        size = static_cast<size_t>(len);
    }

    std::string ret;
    ret.assign(m_ptr->data(), size);
    return ret;
}

std::string Buffer::toHex() const
{
    return Hex::encode(*this);
}

Buffer Buffer::fromHex(const std::string& hex)
{
    return Hex::decode(hex);
}

std::string Buffer::toBase64() const
{
    return Base64::encode(*this);
}

Buffer Buffer::fromBase64(const std::string& base64)
{
    return Base64::decode(base64);
}

std::string Hex::encode(const Buffer& buffer)
{
    std::string hexStr;
    auto size = buffer.size();
    auto data = buffer.data();

    for (int i = 0; i < size; ++i) {
        hexStr += Chars[(data[i] & 0xf0) >> 4];
        hexStr += Chars[(data[i] & 0x0f) >> 0];
    }

    return hexStr;
}

Buffer Hex::decode(const std::string& hex)
{
    Buffer buffer;
    std::string hexStr;
    char* data;
    char ch;
    size_t i = 0;
    int j = 0;

    auto size = static_cast<int>(hex.size());
    if (size % 2 != 0) {
        return buffer;
    }

    buffer.resize(size / 2);
    data = buffer.data();
    for (i = 0; i < size; i += 2) {
        ch = hexToUpper(hex.at(i));
        if (!isHex(ch)) {
            break;
        }
        data[j++] = static_cast<char>(Chars.find(hex.at(i)) << 4 | Chars.find(hex.at(i + 1)));
    }

    return buffer;
}

std::string Base64::encode(const Buffer& buffer)
{
    std::string ret;
    int i = 0;
    int j = 0;
    char charArray_3[3];
    char charArray_4[4];

    auto bytesToEncode = buffer.data();
    auto inLen = buffer.size();

    while (inLen--) {
        charArray_3[i++] = *(bytesToEncode++);
        if (i == 3) {
            charArray_4[0] = (charArray_3[0] & 0xfc) >> 2;
            charArray_4[1] = ((charArray_3[0] & 0x03) << 4) + ((charArray_3[1] & 0xf0) >> 4);
            charArray_4[2] = ((charArray_3[1] & 0x0f) << 2) + ((charArray_3[2] & 0xc0) >> 6);
            charArray_4[3] = charArray_3[2] & 0x3f;

            for (i = 0; i < 4; ++i) {
                ret += Chars[charArray_4[i]];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; ++j) {
            charArray_3[j] = '\0';
        }

        charArray_4[0] = (charArray_3[0] & 0xfc) >> 2;
        charArray_4[1] = ((charArray_3[0] & 0x03) << 4) + ((charArray_3[1] & 0xf0) >> 4);
        charArray_4[2] = ((charArray_3[1] & 0x0f) << 2) + ((charArray_3[2] & 0xc0) >> 6);
        charArray_4[3] = charArray_3[2] & 0x3f;

        for (j = 0; j < i + 1; ++j) {
            ret += Chars[charArray_4[j]];
        }

        while (i++ < 3) {
            ret += '=';
        }
    }

    return ret;
}

Buffer Base64::decode(const std::string& base64)
{
    size_t inLen = base64.size();
    int i = 0;
    int j = 0;
    int in = 0;
    char charArray_4[4], charArray_3[3];
    std::string ret;

    while (inLen-- && (base64[in] != '=') && isBase64(base64[in])) {
        charArray_4[i++] = base64[in];
        ++in;

        if (i == 4) {
            for (i = 0; i < 4; ++i) {
                charArray_4[i] = static_cast<char>(Chars.find(charArray_4[i]));
            }

            charArray_3[0] = (charArray_4[0] << 2) + ((charArray_4[1] & 0x30) >> 4);
            charArray_3[1] = ((charArray_4[1] & 0xf) << 4) + ((charArray_4[2] & 0x3c) >> 2);
            charArray_3[2] = ((charArray_4[2] & 0x3) << 6) + charArray_4[3];

            for (i = 0; (i < 3); ++i) {
                ret += charArray_3[i];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; ++j) {
            charArray_4[j] = 0;
        }

        for (j = 0; j < 4; ++j) {
            charArray_4[j] = static_cast<char>(Chars.find(charArray_4[j]));
        }

        charArray_3[0] = (charArray_4[0] << 2) + ((charArray_4[1] & 0x30) >> 4);
        charArray_3[1] = ((charArray_4[1] & 0xf) << 4) + ((charArray_4[2] & 0x3c) >> 2);
        charArray_3[2] = ((charArray_4[2] & 0x3) << 6) + charArray_4[3];

        for (j = 0; (j < i - 1); ++j) {
            ret += charArray_3[j];
        }
    }

    return Buffer{ ret };
}

template<typename T>
BufferWriter& write(BufferWriter& writer, T data)
{
    auto v = crypto::toBigEndian(data);
    return writer.write((const char*)&v, sizeof(T));
}

BufferWriter::BufferWriter(Buffer& buffer)
    : m_buffer{ buffer }
{

}

BufferWriter::~BufferWriter()
{

}

BufferWriter& BufferWriter::write(const char* data, int len)
{
    m_buffer.append(data, len);
    return *this;
}

BufferWriter& BufferWriter::operator<<(uint8_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(uint16_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(uint32_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(uint64_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(int8_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(int16_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(int32_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(int64_t value)
{
    return crypto::write(*this, value);
}

BufferWriter& BufferWriter::operator<<(const char* value)
{
    m_buffer.append(value);
    return *this;
}

BufferWriter& BufferWriter::operator<<(const std::string& value)
{
    m_buffer.append(value);
    return *this;
}

BufferWriter& BufferWriter::operator<<(const Buffer& value)
{
    m_buffer.append(value);
    return *this;
}

template<typename T>
BufferReader& read(BufferReader& reader, T& data)
{
    if (reader.read((char*)&data, sizeof(T)) == sizeof(T)) {
        data = crypto::fromBigEndian(data);
    }
    return reader;
}

BufferReader::BufferReader(const Buffer& buffer)
    : m_buffer{ buffer }
    , m_position{ 0 }
{

}

BufferReader::~BufferReader()
{

}

int BufferReader::read(char* buffer, int len)
{
    auto size = m_buffer.size();
    if (m_position >= size) {
        return 0;
    }

    auto count = len;
    auto left = size - m_position;
    if (len > left) {
        count = left;
    }

    std::memcpy(buffer, m_buffer.data() + m_position, count);
    m_position += count;

    return count;
}

int BufferReader::seek(int position)
{
    auto tmp = m_position;
    auto size = m_buffer.size();

    auto np = position;
    if (np < 0) {
        np = 0;
    }
    else if (np > size) {
        np = size;
    }

    m_position = np;
    return tmp;
}

int BufferReader::position() const
{
    return m_position;
}

bool BufferReader::atEnd() const
{
    return (m_position >= m_buffer.size());
}

BufferReader::operator bool() const
{
    return !atEnd();
}

BufferReader& BufferReader::operator>>(uint8_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(uint16_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(uint32_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(uint64_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(int8_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(int16_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(int32_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(int64_t& value)
{
    return crypto::read(*this, value);
}

BufferReader& BufferReader::operator>>(std::string& value)
{
    auto tmp = m_position;
    auto size = m_buffer.size();
    auto data = m_buffer.data();

    while (tmp < size) {
        if (data[tmp] == 0 || data[tmp] == '\0') {
            break;
        }
        ++tmp;
    }

    if (tmp > m_position) {
        value.assign(data + m_position, tmp - m_position);
        m_position = tmp + 1;
    }

    return *this;
}

BufferReader& BufferReader::operator>>(Buffer& buffer)
{
    read(buffer.data(), buffer.size());
    return *this;
}

std::ofstream& operator <<(std::ofstream& stream, const crypto::Buffer& buffer)
{
    stream.write(buffer.data(), buffer.size());
    return stream;
}

std::ifstream& operator>>(std::ifstream& stream, crypto::Buffer& buffer)
{
    stream.read(buffer.data(), buffer.size());
    buffer.resize(static_cast<int>(stream.gcount()));
    return stream;
}

std::fstream& operator<<(std::fstream& stream, const crypto::Buffer& buffer)
{
    stream.write(buffer.data(), buffer.size());
    return stream;
}

std::fstream& operator>>(std::fstream& stream, crypto::Buffer& buffer)
{
    stream.read(buffer.data(), buffer.size());
    buffer.resize(static_cast<int>(stream.gcount()));
    return stream;
}

