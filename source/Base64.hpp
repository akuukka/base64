#pragma once
#include <stdexcept>
#include <cstddef>

namespace base64
{

struct InvalidEncoding : std::runtime_error {
    InvalidEncoding(const char* what) : std::runtime_error(what) {}
};

template<typename T>
inline T encode(const void* data, size_t count)
{
    T r;
    const size_t padding = (3 - count % 3) % 3;
    const size_t bits = (count + padding) * 8 - padding * 6;
    const size_t sextets = bits / 6;
    r.resize(sextets + padding);
    for (size_t i=0; i < sextets; i++) {
        size_t v = 0;
        for (size_t j=0;j<6;j++) {
            const int bit = i * 6 + j;
            const size_t bval =
                bit/8 < count ? (static_cast<const char*>(data)[bit/8] & (1<<(7-(bit%8)))) : 0;
            if (bval) {
                v = v | (1 << (5-j));
            }
        }
        r[i] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[v];
    }
    for (size_t i=0; i < padding; i++) {
        r[sextets + i] = '=';
    }
    return r;
}

template<typename T>
inline T decode(const char* data, size_t count)
{
    T r;
    const int table[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1,
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
    r.resize(bytes);
    for (size_t i=0;i<count;i++) {
        const char c = data[i];
        for (size_t j=0;j<6;j++) {
            const int bit = i * 6 + j;
            if (bit >= bits) {
                break;
            }
            if (table[c] == -1) {
                throw InvalidEncoding("Unexpected character in encoding.");
            }
            if (table[c] & (1 << (5-j))) {
                r[bit/8] = r[bit/8] | (1 << (7-bit%8));
            }
            else {
                r[bit/8] = r[bit/8] & ~(1 << (7-bit%8));
            }
        }
    }
    return r;
}

}
