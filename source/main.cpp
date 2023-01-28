#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include "Base64.hpp"

int main(int argc, char** argv)
{
    struct TestCase {
        const char* data;
        const char* encoded;
    };
    TestCase cases[] = {
        {"light work", "bGlnaHQgd29yaw=="},
        {"Man", "TWFu"},
        {"fdas", "ZmRhcw=="},
        {"", ""},
        {"Ma", "TWE="},
        {"M", "TQ=="},
        {"light wor", "bGlnaHQgd29y"},
        {"light work.", "bGlnaHQgd29yay4="},
    };
    for (const auto& testCase : cases) {
        const auto decoded =
            base64::decode<std::string>(testCase.encoded);
        if (decoded != testCase.data) {
            std::cerr << "Expected base64 decoding of " << testCase.encoded
                      << " to be " << testCase.data << ", but got "
                      << decoded << " instead.\n";
            return 1;
        }
        const auto encoded = base64::encode(testCase.data, strlen(testCase.data));
        if (encoded != testCase.encoded) {
            std::cerr << "Expected base64 encoding of " << testCase.data
                      << " to be " << testCase.encoded << ", but got "
                      << encoded << " instead.\n";
            return 1;
        }
    }
    
    std::string data;
    data.resize(25);
    for (int i=0; i < 100000; i++) {
        size_t len = rand() % 10;
        for (size_t i=0; i < 25; i++) {
            data[i] = i < len ? rand() % 256 : 0xff;
        }
        const std::string encoded = base64::encode(&data[0], len);
        const std::string decoded = base64::decode<std::string>(encoded);
        if (decoded != data.substr(0, len)) {
            std::cerr << "Didn't get original back.\n";
            return 1;
        }
    }
    
    struct PodStruct {
        int i;
        double f;
        char str[5];
    } a, b;
    a.i = 21;
    a.f = -4214.1411;
    strcpy(a.str, "test");
    
    const std::string encoded = base64::encode(&a, sizeof(a));
    base64::decode(encoded, &b);
    const std::string encoded2 = base64::encode(&b, sizeof(b));
    assert(encoded2 == encoded);
    if (encoded != encoded2) {
        std::cout << "POD struct changed.";
        return 1;
    }

    const char* bad[] = {
        "A=AA"
    };
    for (const auto b : bad) {
        bool exception = false;
        try {
            base64::decode<std::string>(b);
        }
        catch (base64::InvalidEncoding&) {
            exception = true;
        }
        if (!exception) {
            std::cerr << b << " didn't cause exception, but should have so.";
            return 1;
        }
    }

    // Same buffer for input and output is acceptable when decoding (but don't try
    // when encoding...)
    {
        const std::string data = "whatever";
        std::string buffer = base64::encode(data.c_str(), data.size());
        const size_t bytes = base64::decode(buffer, &buffer[0]);
        buffer.resize(bytes);
        assert(buffer == data);
    }

    // Check that junk beyond the bounds of our buffer does not affect the end result:
    {
        std::string buf, buf2;
        buf.resize(3);
        buf2.resize(3);
        buf[0] = 'M';
        buf[1] = 0xff;
        buf[2] = 0xff;
        buf2[0] = 'M';
        buf2[1] = 0;
        buf2[2] = 0;;
        if (base64::encode(&buf[0], 1) != base64::encode(&buf2[0], 1)) {
            std::cerr << "Junk beyounds bounds does matter.\n";
            return 1;
        }
    }
    {
        std::string buf, buf2;
        buf.resize(3);
        buf2.resize(3);
        buf[0] = 'M';
        buf[1] = 'a';
        buf[2] = 0xff;
        buf2[0] = 'M';
        buf2[1] = 'a';
        buf2[2] = 0;
        if (base64::encode(&buf[0], 2) != base64::encode(&buf2[0], 2)) {
            std::cerr << "Junk beyounds bounds does matter.\n";
            return 1;
        }
    }

    // Check that writing beyond buffer bounds doesn't occur.
    {
        std::vector<unsigned char> buf;
        // "M", "TQ=="
        buf.resize(base64::dataLength("TQ==", 4) + 1);
        assert(buf.size() == 2);
        buf[1] = 0xff;
        base64::decode("TQ==", 4, &buf[0]);
        assert(buf[0]=='M');
        if (buf[1] != 0xff) {
            std::cerr << "Wrote beyond buffer bounds\n";
        }
    }
    return 0;
}
