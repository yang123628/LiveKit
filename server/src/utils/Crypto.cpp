#include "utils/Crypto.h"
#include <openssl/sha.h>
#include <fstream>
#include <random>
#include <iomanip>
#include <sstream>

std::string Crypto::sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

std::string Crypto::generateSalt(size_t length) {
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    std::vector<unsigned char> buf(length);
    if (urandom.is_open()) {
        urandom.read(reinterpret_cast<char*>(buf.data()), length);
        urandom.close();
    } else {
        std::random_device rd;
        for (size_t i = 0; i < length; ++i) {
            buf[i] = static_cast<unsigned char>(rd());
        }
    }
    return bytesToHex(buf.data(), length);
}

std::string Crypto::hashPassword(const std::string& password, const std::string& salt) {
    return sha256(password + salt);
}

std::string Crypto::bytesToHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}
