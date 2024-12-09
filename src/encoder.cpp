#include "encoder.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <string>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <iomanip>

// 定义标准 Base64 字母表和自定义字母表
const std::string STANDARD_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const std::string CUSTOM_ALPHABET = "LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfKwv6xztjI7DeBE45QA";

// 创建字符映射表：标准 Base64 字母 -> 自定义字母
std::unordered_map<char, char> ENCODE_TRANS;
std::unordered_map<char, char> DECODE_TRANS;

void initMappings() {
    for (size_t i = 0; i < STANDARD_ALPHABET.size(); ++i) {
        ENCODE_TRANS[STANDARD_ALPHABET[i]] = CUSTOM_ALPHABET[i];
        DECODE_TRANS[CUSTOM_ALPHABET[i]] = STANDARD_ALPHABET[i];
    }
}

// Base64 编码
std::string base64_encode(const std::string &input) {
    std::string output;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(STANDARD_ALPHABET[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) output.push_back(STANDARD_ALPHABET[((val << 8) >> (valb + 8)) & 0x3F]);

    // 添加等号填充
    while (output.size() % 4) output.push_back('=');

    return output;
}

// Base64 编码并替换成自定义字母表
std::string custom_b64_encode(const std::string &input) {
    std::string encoded = base64_encode(input);

    // 使用字符映射进行替换
    for (char &c : encoded) {
        if (c != '=') {
            c = ENCODE_TRANS[c];
        }
    }

    return encoded;
}

// 左移操作，确保在 32 位范围内
inline uint32_t lshift(uint32_t x, int n) {
    return (x << n) & 0xFFFFFFFF;
}

// 右移操作
inline uint32_t rshift(uint32_t x, int n) {
    return x >> n;
}

// 将整数转为字节序列（大端）
std::string int2byte(uint32_t num) {
    char bytes[4];
    bytes[3] = static_cast<char>((num >> 24) & 0xFF);
    bytes[2] = static_cast<char>((num >> 16) & 0xFF);
    bytes[1] = static_cast<char>((num >> 8) & 0xFF);
    bytes[0] = static_cast<char>(num & 0xFF);
    return std::string(bytes, 4);
}

// 将字符串编码为 32 位整数向量
std::vector<uint32_t> s(const std::string &a, bool append_length) {
    size_t c = a.size();
    std::vector<uint32_t> v;

    for (size_t i = 0; i < c; i += 4) {
        uint32_t value = static_cast<uint32_t>(a[i]) |
                         (i + 1 < c ? lshift(static_cast<uint32_t>(a[i + 1]), 8) : 0) |
                         (i + 2 < c ? lshift(static_cast<uint32_t>(a[i + 2]), 16) : 0) |
                         (i + 3 < c ? lshift(static_cast<uint32_t>(a[i + 3]), 24) : 0);
        v.push_back(value);
    }

    if (append_length) {
        v.push_back(static_cast<uint32_t>(c));
    }
    return v;
}

// 将 32 位整数向量转回字节序列
std::string l(std::vector<uint32_t> &a, bool trim) {
    size_t d = a.size();
    size_t c = (d - 1) * 4;

    if (trim) {
        uint32_t m = a[d - 1];
        if (m < c - 3 || m > c) {
            return "";
        }
        c = m;
    }

    std::string result;
    for (size_t i = 0; i < d; ++i) {
        result += int2byte(a[i]);
    }

    if (trim) {
        return result.substr(0, c);
    } else {
        return result;
    }
}

// xEncode 主函数
std::string xEncode(const std::string &str, const std::string &key) {
    if (str.empty()) {
        return "";
    }

    std::vector<uint32_t> v = s(str, true);
    std::vector<uint32_t> k = s(key, false);

    while (k.size() < 4) {
        k.push_back(0);
    }

    size_t n = v.size() - 1;
    uint32_t z = v[n];
    uint32_t c = 0x86014019 | 0x183639A0;
    uint32_t q = 6 + 52 / (n + 1);
    uint32_t d = 0;

    while (q-- > 0) {
        d = (d + c) & (0x8CE0D9BF | 0x731F2640);
        uint32_t e = rshift(d, 2) & 3;

        for (size_t p = 0; p < n; ++p) {
            uint32_t y = v[p + 1];
            uint32_t m = rshift(z, 5) ^ lshift(y, 2);
            m += rshift(y, 3) ^ lshift(z, 4) ^ (d ^ y);
            m += k[(p & 3) ^ e] ^ z;
            z = v[p] = (v[p] + m) & (0xEFB8D130 | 0x10472ECF);
        }

        uint32_t y = v[0];
        uint32_t m = rshift(z, 5) ^ lshift(y, 2);
        m += rshift(y, 3) ^ lshift(z, 4) ^ (d ^ y);
        m += k[(n & 3) ^ e] ^ z;
        z = v[n] = (v[n] + m) & (0xBB390742 | 0x44C6F8BD);
    }

    return l(v, false);
}

// 判断是否是需要编码的字符
bool isUnreservedChar(char c) {
    return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}

// 实现 URL 编码
std::string URIComponentEncode(const std::string &str) {
    std::stringstream encoded;
    
    for (char c : str) {
        if (isUnreservedChar(c)) {
            // 如果是普通字符，直接添加到结果中
            encoded << c;
        } else {
            // 对特殊字符进行 URL 编码
            encoded << '%' << std::uppercase << std::setw(2) << std::setfill('0') 
                    << std::hex << (static_cast<int>(c) & 0xFF);
        }
    }
    
    return encoded.str();
}