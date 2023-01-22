#pragma once
#include <string>
#include <stdexcept>
#include <cstddef>

namespace base64
{

struct InvalidEncoding : std::runtime_error {
    InvalidEncoding(const char* what) : std::runtime_error(what) {}
};

template<typename T>
inline T encode(const void* data, size_t bytes)
{
    T r;
    const auto masks = 51331068ULL;
    const auto d = static_cast<const unsigned char*>(data);
    const size_t padding = (3 - bytes % 3) % 3;
    const size_t bits = (bytes + padding) * 8 - padding * 6;
    const size_t sextets = bits / 6;
    r.resize(sextets + padding);
    for (size_t i=0; i < sextets; i++) {
        size_t beginBit = i * 6;
        size_t endBit = (i+1) * 6;
        size_t beginByte = beginBit / 8;
        size_t endByte = endBit / 8;
        int beginOffset = beginBit % 8;
        int endOffset = endBit % 8;
        const int mask1 = reinterpret_cast<const unsigned char*>(&masks)[beginOffset/2];
        int v = (d[beginByte] & mask1) >> (beginOffset == 0 ? 2 : 0);
        if (endByte > beginByte) {
            v = v << endOffset;
            if (endByte < bytes) {
                const int mask2 = 0x3f << (6 - endOffset);
                v = v | (d[endByte] & mask2) >> (8 - endOffset);
            }
        }
        r[i] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[v];
    }
    for (size_t i=0; i < padding; i++) {
        r[sextets + i] = '=';
    }
    return r;
}

inline int shift(int v, int n)
{
    if (n >= 0) {
        return v << n;
    }
    return v >> -n;
}

/// Return number of bytes represented by a base64 encoded string.
inline size_t dataLength(const char* base64String, size_t length, size_t* paddingOut = nullptr)
{
    const size_t padding =
        (length >= 1 && base64String[length - 1] == '=') ?
        (length >= 2 && base64String[length - 2] == '=' ? 2 : 1) : 0;
    const size_t bits = length ? length * 6 - padding * 8 : 0;
    const size_t bytes = bits / 8;
    if (paddingOut) {
        *paddingOut = padding;
    }
    return bytes;
}

inline size_t decode(const char* str, size_t length, void* out)
{
    const unsigned char* data = reinterpret_cast<const unsigned char*>(str);
    unsigned char* r = static_cast<unsigned char*>(out);
    const unsigned char table[] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255,
        255, 255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
        45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255
    };
    if (!(length % 4 == 0)) {
        throw InvalidEncoding("Padding expected");
    }
    size_t padding;
    const size_t bytes = dataLength(str, length, &padding);
    const size_t sextets = length - padding;
    const unsigned char masks[] = { 3, 192, 240, 252 };
    for (size_t i=0; i < sextets; i++) {
        const unsigned char c = table[data[i]];
        if (c == 255) {
            throw InvalidEncoding("Unexpected character in encoding");
        }
        const size_t beginBit = i * 6;
        const size_t endBit = (i+1) * 6;
        const size_t beginByte = beginBit / 8;
        const size_t endByte = endBit / 8;
        const int beginOffset = beginBit % 8;
        const unsigned char mask1  = masks[beginOffset/2];
        r[beginByte] = r[beginByte] & mask1 | shift(c, 2 - beginOffset);
        if (beginByte != endByte && endByte < bytes) {
            const int endOffset = endBit % 8;
            const unsigned char mask2 = ~(((1 << (8-endOffset)) - 1) << endOffset);
            r[endByte] = r[endByte] & mask2 | (c << (8-endOffset));
        }
    }
    return bytes;
}

inline size_t decode(const std::string& base64, void* out)
{
    return decode(base64.c_str(), base64.size(), out);
}

template<typename T>
inline T decode(const char* data, size_t count)
{
    T r;
    r.resize(dataLength(data, count));
    decode(data, count, &r[0]);
    return r;
}

template<typename T>
inline T decode(const std::string& str)
{
    return decode<T>(str.c_str(), str.length());
}

}
