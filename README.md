# base64
Header only C++ base64 encoding/decoding

To encode any data:

  std::string encoded = base64::encode(ptrToVoid, numBytes);
  
To decode to a container that has resize function and random access using [] operator:

  std::vector<unsigned char> bytes = base64::decode<std::vector<unsigned char>>(encoded);
  std::vector<char> bytes = base64::decode<std::vector<char>>(encoded);
  std::string bytes = base64::decode<std::string>(encoded);
  
To decode to a pre-existing buffer:

  size_t bytes = base64::decode(encoded.c_str(), encoded.size(), out);

You can use dataLength function to check how many bytes of data a base64 encoded string contains.

  size_t bytes = base64::dataLength(encoded.c_str(), encoded.size());
