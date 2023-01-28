#pragma once

/*
MIT License

Copyright (c) 2023 Antti Kuukka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <string>
#include <stdexcept>
#include <cstddef>

namespace base64
{

struct InvalidEncoding : std::runtime_error {
    InvalidEncoding(const char* what) : std::runtime_error(what) {}
};

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

/// Return number of bytes required by the base64 representation of dataLength bytes.
inline size_t encodedLength(size_t dataLength, size_t* paddingOut = nullptr)
{
    const size_t padding = (3 - dataLength % 3) % 3;
    const size_t bits = (dataLength + padding) * 8 - padding * 6;
    const size_t sextets = bits / 6;
    if (paddingOut) {
        *paddingOut = padding;
    }
    return sextets + padding;
}

inline void encodeChunk(const unsigned char d[3], char* out)
{
    const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const std::uint32_t co = (d[2])<<16 | (d[1])<<8 | d[0];
    const std::uint32_t w2 = (co & 0b11111100) >> 2
        | ((co & (0b1111 << 12)) >> 6) | ( (co & 0b11) << 10)
        | (co & (0b11 << 22)) >> 10
        | (co & (0b1111 << 8)) << 6
        | (co & (0b111111 << 16)) << 2;
    out[0] = table[w2 & 63];
    out[1] = table[(w2>>6) & 63];
    out[2] = table[(w2>>12) & 63];
    out[3] = table[(w2>>18) & 63];
}

inline void encode(const void* data, size_t bytes, char* out)
{
    const auto d = static_cast<const unsigned char*>(data);
    size_t padding;
    const size_t sextets = encodedLength(bytes, &padding) - padding;
    const size_t groups = (sextets + 4 - 1) / 4;
    const size_t safeGroups = (bytes % 3 == 0) ? groups : groups - 1;
    for (size_t i=0; i < safeGroups; i++) {
        encodeChunk(&d[i*3], &out[i * 4]);
    }
    const size_t remaining = bytes - safeGroups * 3;
    if (remaining) {
        const unsigned char zero = 0;
        const size_t i = groups - 1;
        const unsigned char last[3] = {d[i*3], remaining == 2 ? d[i*3 + 1] : zero, zero};
        encodeChunk(last, &out[i * 4]);
    }
    for (size_t i=0; i < padding; i++) {
        out[sextets + i] = '=';
    }
}

inline std::string encode(const void* data, size_t bytes)
{
    std::string r;
    r.resize(encodedLength(bytes));
    encode(data, bytes, &r[0]);
    return r;
}

inline size_t dataLength(const std::string& base64String, size_t* paddingOut = nullptr)
{
    return dataLength(base64String.c_str(), base64String.size(), paddingOut);
}

inline void decodeChunk(const char d[4], std::uint8_t out[3])
{
    const std::uint8_t table[] = {
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
    if ((table[d[0]] | table[d[1]] | table[d[2]] | table[d[3]]) & (1<<7)) {
        throw InvalidEncoding("Invalid character found in chunk");
    }
    out[0] = table[d[0]] << 2 | ((table[d[1]] & 0b110000) >> 4);
    out[1] = (table[d[1]] & 0b1111) << 4 | (table[d[2]] & 0b111100) >> 2;
    out[2] = (table[d[2]] & 0b11) << 6 | (table[d[3]] & 0b111111);
}

inline size_t decode(const char* str, size_t length, void* out)
{
    if (!(length % 4 == 0)) {
        throw InvalidEncoding("Padding expected");
    }
    unsigned char* r = static_cast<unsigned char*>(out);
    size_t padding;
    const size_t bytes = dataLength(str, length, &padding);
    const size_t chunks = (length + 4 - 1) / 4;
    const size_t safeChunks = (bytes % 3 == 0) ? chunks : chunks - 1;
    for (size_t i=0; i < safeChunks; i++) {
        decodeChunk(&str[i* 4], &r[i * 3]);
    }
    if (safeChunks < chunks) {
        const char chunk[4] = {
            str[safeChunks * 4 + 0], str[safeChunks * 4 + 1],
            padding == 1 ? str[safeChunks * 4 + 2] : 'A', 'A'
        };
        std::uint8_t r2[3];
        decodeChunk(chunk, r2);
        r[safeChunks * 3] = r2[0];
        if (padding == 1) {
            r[safeChunks * 3 + 1] = r2[1];
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
    return decode<T>(str.length() ? &str[0] : "", str.length());
}

}
