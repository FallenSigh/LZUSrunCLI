#include "utils.h"
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <iomanip>
#include <regex>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

#ifdef __linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#endif

std::string unwrap_callback(const std::string& response, const std::string& callback) {
    return response.substr(callback.size() + 1, response.size() - callback.size() - 2);
}

std::string parse_csrf_token(const std::string& html) {
    std::regex pattern(R"(<meta\s+name=\"csrf-token\"\s+content=\"([^\"]+)\">)");
    std::smatch matches;
    std::string res;
    // 查找匹配的内容
    if (std::regex_search(html, matches, pattern)) {
        // 第一个捕获组是 content 的值
        res = matches[1];
    } 
    return res;
}

std::string sha1_hash(const std::string &input) {
    unsigned char hash[SHA_DIGEST_LENGTH];  // SHA-1 的哈希值长度是 20 字节

    // 计算 SHA-1 哈希
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    // 将结果转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)hash[i];
    }

    return ss.str();
}

std::string hmac_md5(const std::string &key, const std::string &message) {
    unsigned char* result;
    unsigned int len = MD5_DIGEST_LENGTH;

    // 创建 HMAC 的 context，使用 MD5 算法
    result = HMAC(EVP_md5(), key.c_str(), key.length(), 
                  (unsigned char*)message.c_str(), message.length(), NULL, &len);

    // 将结果转换为十六进制字符串
    std::stringstream ss;
    for (unsigned int i = 0; i < len; ++i) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)result[i];
    }
    
    return ss.str();
}

unsigned long long get_random_17() {
    std::random_device rd;  
    std::mt19937 gen(rd()); 

    std::uniform_int_distribution<unsigned long long> dist(10000000000000000ULL, 99999999999999999ULL);

    unsigned long long random_number = dist(gen);
    return random_number;
};

long long get_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()).count();
    return timestamp;
};

std::string generate_callback() {
    return "jQuery" + std::to_string(get_random_17()) + "_" + std::to_string(get_timestamp_ms());
}

nlohmann::json get_os() {
    return {
        {"device", "Android"},
        {"platform", "Smartphones/PDAs/Tablets"}
    };
}

#ifdef __linux
std::string get_ip_address() {
    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *tempAddr = nullptr;
    std::string ipAddress;

    // 获取网络接口信息
    if (getifaddrs(&interfaces) == -1) {
        perror("getifaddrs");
        return "";
    }

    // 遍历所有网络接口
    tempAddr = interfaces;
    while (tempAddr != nullptr) {
        if (tempAddr->ifa_addr && tempAddr->ifa_addr->sa_family == AF_INET) { // 只处理 IPv4 地址
            char ip[INET_ADDRSTRLEN];
            void* addr = &((struct sockaddr_in *)tempAddr->ifa_addr)->sin_addr;

            // 转换地址为字符串
            inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);

            // 忽略回环地址 127.0.0.1
            if (std::string(ip) != "127.0.0.1") {
                ipAddress = ip;
                break; // 找到第一个非回环地址后退出
            }
        }
        tempAddr = tempAddr->ifa_next;
    }

    // 释放接口信息内存
    freeifaddrs(interfaces);

    return ipAddress;
}
#elifdef _WIN32
std::string get_ip_address() {
    WSADATA wsaData;
    char hostname[256];
    struct hostent *host_entry;
    char *ip_address;

    // 初始化Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // std::cerr << "WSAStartup failed!" << std::endl;
        return "0.0.0.0";
    }

    // 获取本机主机名
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        // std::cerr << "gethostname failed!" << std::endl;
        WSACleanup();
        return "0.0.0.0";
    }

    // 获取主机信息
    host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        // std::cerr << "gethostbyname failed!" << std::endl;
        WSACleanup();
        return "0.0.0.0";
    }

    // 获取IP地址
    ip_address = inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);
    WSACleanup();
    return std::string(ip_address);
}

#endif