#pragma once

#include <string>
#include <nlohmann/json.hpp>

std::string get_ip_address();
nlohmann::json get_os();
unsigned long long get_random_17();
long long get_timestamp_ms();
std::string generate_callback();
std::string hmac_md5(const std::string &key, const std::string &message);
std::string sha1_hash(const std::string &input);
std::string parse_csrf_token(const std::string& html);
std::string unwrap_callback(const std::string& response, const std::string& callback);