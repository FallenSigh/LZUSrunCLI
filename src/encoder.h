#pragma once
#include <string>

std::string xEncode(const std::string &str, const std::string &key);
std::string URIComponentEncode(const std::string &str);
std::string base64_encode(const std::string &input);
std::string custom_b64_encode(const std::string &input);
void initMappings();