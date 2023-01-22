#include <iostream>
#include <string>
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
        const auto encoded = base64::encode<std::string>(testCase.data, strlen(testCase.data));
        if (encoded != testCase.encoded) {
            std::cerr << "Expected base64 encoding of " << testCase.data
                      << " to be " << testCase.encoded << ", but got "
                      << encoded << " instead.\n";
            return 1;
        }
    }
    std::string data;
    for (int i=0; i < 10000;i++) {
        size_t len = (rand() % 25);
        data.resize(len);
        for (size_t i=0;i<len;i++) {
            data[i] = rand() % 256;
        }
        const std::string encoded = base64::encode<std::string>(data.c_str(), data.length());
        const std::string decoded = base64::decode<std::string>(encoded);
        if (decoded != data) {
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
    
    const std::string encoded = base64::encode<std::string>(&a, sizeof(a));
    base64::decode(encoded, &b);
    const std::string encoded2 = base64::encode<std::string>(&b, sizeof(b));
    assert(encoded2 == encoded);
    if (encoded != encoded2) {
        std::cout << "POD struct changed.\n";
        return 1;
    }
    return 0;
}
