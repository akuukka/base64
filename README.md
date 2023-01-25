# base64
Header only C++ base64 encoding/decoding

MIT License

To encode any data:

    std::string encoded = base64::encode(ptrToVoid, numBytes);

To decode to a container that has resize function and random access using [] operator:

    std::vector<unsigned char> bytes = base64::decode<std::vector<unsigned char>>(encoded);
    std::vector<char> bytes = base64::decode<std::vector<char>>(encoded);
    std::string bytes = base64::decode<std::string>(encoded);
  
To decode to a pre-existing buffer:

    // void* out is a pointer to a buffer of data which has to be large enough
    // to hold the decoded data. See discussion about dataLength function below
    // to see how you can get the minimum size for your buffer.
    size_t bytes = base64::decode(encoded, out);
    // Or if you are not using std::string:
    size_t bytes = base64::decode(encoded.c_str(), encoded.size(), out);

You can use dataLength function to check how many bytes of data a base64 encoded string contains.

    size_t bytes = base64::dataLength(encoded);
    // And again, you don't need to use std::string:
    size_t bytes = base64::dataLength(encoded.c_str(), encoded.size());
