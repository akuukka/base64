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

inline size_t decode(const char* data, size_t count, void* out)
{
    unsigned char* r = static_cast<unsigned char*>(out);
    const int table[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 255, -1,
        -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
        22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
        38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
    if (!(count % 4 == 0)) {
        throw InvalidEncoding("Padding expected");
    }
    const size_t padding =
        (count >= 1 && data[count - 1] == '=') ?
        (count >= 2 && data[count - 2] == '=' ? 2 : 1) : 0;
    const size_t bits = count ? count * 6 - padding * 8 : 0;
    const size_t bytes = bits / 8;
    const size_t sextets = count - padding;
    const unsigned char masks[] = { 3, 192, 240, 252 };
    for (size_t i=0; i < sextets; i++) {
        const char c = data[i];
        if (table[c] == -1) {
            throw InvalidEncoding((std::string("Unexpected character in encoding: ") + c).c_str());
        }
        const size_t beginBit = i * 6;
        const size_t endBit = (i+1) * 6;
        const size_t beginByte = beginBit / 8;
        const size_t endByte = endBit / 8;
        const int beginOffset = beginBit % 8;
        const int endOffset = endBit % 8;
        const unsigned char mask1  = masks[beginOffset/2];
        r[beginByte] = r[beginByte] & mask1 | shift(table[c], 2 - beginOffset);
        if (beginByte != endByte && endByte < bytes) {
            const unsigned char mask2 = ~(((1 << (8-endOffset)) - 1) << endOffset);
            r[endByte] = r[endByte] & mask2 | (table[c] << (8-endOffset));
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
    const size_t padding =
        (count >= 1 && data[count - 1] == '=') ?
        (count >= 2 && data[count - 2] == '=' ? 2 : 1) : 0;
    const size_t bits = count ? count * 6 - padding * 8 : 0;
    const size_t bytes = bits / 8;
    T r;
    r.resize(bytes);
    decode(data, count, &r[0]);
    return r;
}

template<typename T>
inline T decode(const std::string& str)
{
    return decode<T>(str.c_str(), str.length());
}

}
