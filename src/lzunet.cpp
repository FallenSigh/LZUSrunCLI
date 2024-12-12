#include "lzunet.h"
#include <cpr/cpr.h>
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/parameters.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "utils.h"
#include "encoder.h"
#include <exception>
#include <format>
#include <iostream>
#include <regex>

constexpr const char* CHALLENGE_URL = "http://10.10.0.166/cgi-bin/get_challenge?callback={}&username={}&ip={}&_={}";
constexpr const char* LOGIN_URL = "http://10.10.0.166/cgi-bin/srun_portal";
constexpr const char* RAD_USER_INFO_URL = "http://10.10.0.166/cgi-bin/rad_user_info";
constexpr const char *HOME_URL = "http://10.10.0.166:8800/site/sso";

constexpr const char* ENC_VER = "srun_bx1";
constexpr const char* AC_ID = "2";
constexpr const char* N = "200";
constexpr const char* TYPE = "1";

using unordered_json = nlohmann::basic_json<
    nlohmann::ordered_map, // 替换为 std::unordered_map
    std::vector, std::string, bool, 
    std::int64_t, std::uint64_t, double, 
    std::allocator, nlohmann::adl_serializer>;

LZUNetwork::LZUNetwork(const std::string& account, const std::string& password) {
    this->_password = base64_encode(URIComponentEncode(password));
    this->_account = account;
    auto p = account.find('@');
    this->_username = account.substr(0, p);
    this->_ip = get_ip_address();
}

std::string LZUNetwork::get_challenge() {
    auto callback = generate_callback();
    auto url = std::format(CHALLENGE_URL, callback, this->_account, this->_ip, std::to_string(get_timestamp_ms()));
   
    auto response = cpr::Get(cpr::Url{url});
    auto result = unwrap_callback(response.text, callback);

    std::string challenge = "";
    try {
        unordered_json res = nlohmann::json::parse(result);
        challenge = res["challenge"];
    }
    catch (const std::exception& e) {
        std::cerr << "parse error: " << e.what() << "\n";
    }

    return challenge;
}

std::string LZUNetwork::login() {
    auto challenge = get_challenge();
    auto token = challenge;

    unordered_json j = {
        {"username", _account},
        {"password", _password},
        {"ip", _ip},
        {"acid", AC_ID},
    {"enc_ver", ENC_VER}
    };
    std::string json_str = j.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
    std::string i = "{SRBX1}" + custom_b64_encode(xEncode(json_str, token));

    std::string hmd5 = hmac_md5(token, "");
    std::string chkstr = token + _account;
    chkstr += token + hmd5;
    chkstr += token + AC_ID;
    chkstr += token + _ip;
    chkstr += token + N;
    chkstr += token + TYPE;
    chkstr += token + i;
    auto os = get_os();

    std::string callback = generate_callback();
    auto params = cpr::Parameters{
        {"callback", callback},
        {"action", "login"},
        {"username", _account},
        {"password", "{MD5}" + hmd5},
        {"ac_id", AC_ID},
        {"ip", _ip},
        {"chksum", sha1_hash(chkstr)},
        {"info", i},
        {"n", N},
        {"type", TYPE},
        {"os", std::string(os["device"])},
        {"name", std::string(os["platform"])},
        {"double_stack", "0"},
        {"_", std::to_string(get_timestamp_ms() + 3)}};

    auto response = cpr::Get(cpr::Url{LOGIN_URL}, params);
    auto result = unwrap_callback(response.text, callback);
    return result;
}

std::string LZUNetwork::logout() {
    std::string callback = generate_callback();
    auto params = cpr::Parameters{
        {"callback", callback},
        {"action", "logout"},
        {"username", _account},
        {"ip", _ip},
        {"ac_id", AC_ID},
        {"_", std::to_string(get_timestamp_ms())}
    };

    auto response = cpr::Get(cpr::Url{LOGIN_URL}, params);
    auto result = unwrap_callback(response.text, callback);
    return result;
}

std::string LZUNetwork::status() {
    std::string data = base64_encode(_username + ":" + _password);
    auto params = cpr::Parameters{{"data", data}};
    auto response = cpr::Get(cpr::Url{HOME_URL}, params);
    
    std::regex regex_pattern(R"(<td data-col-seq="0">(.*?)</td><td data-col-seq="1">(.*?)</td><td data-col-seq="2">(.*?)</td><td data-col-seq="5">(.*?)</td>)");
    std::smatch match;

    auto res = nlohmann::json::array();

    std::regex_iterator<std::string::iterator> rit(response.text.begin(), response.text.end(), regex_pattern);
    std::regex_iterator<std::string::iterator> rend;

    while (rit != rend) {
        std::string username = rit->str(1);
        std::string ip_address = rit->str(2);
        std::string online_time = rit->str(3);
        std::string product_name = rit->str(4);

        res.push_back({
            {"username", username},
            {"ip", ip_address},
            {"online_time", online_time},
            {"product_name", product_name}
        });

        ++rit;  
    }

    return res.dump();
}

std::string LZUNetwork::info() {
    std::string data = base64_encode(_username + ":" + _password);
    auto params = cpr::Parameters{{"data", data}};
    auto response = cpr::Get(cpr::Url{HOME_URL}, params);
    
    std::regex regex_pattern(R"(<td data-col-seq="1">(.*?)</td><td data-col-seq="3">(.*?)</td><td data-col-seq="4">(.*?)</td><td data-col-seq="5">(.*?)</td><td data-col-seq="7">(.*?)</td>)");
    std::smatch match;
    auto res = nlohmann::json::array();

    std::regex_iterator<std::string::iterator> rit(response.text.begin(), response.text.end(), regex_pattern);
    std::regex_iterator<std::string::iterator> rend;

    while (rit != rend) {
        std::string product_name = rit->str(1);
        std::string used_traffic = rit->str(2);
        std::string used_time = rit->str(3);
        std::string usage_count = rit->str(4);
        std::string balance = rit->str(5);

        res.push_back({
            {"product_name", product_name},
            {"used_traffic", used_traffic},
            {"used_time", used_time},
            {"usage_count", usage_count},
            {"balance", balance}
        });

        ++rit;  
    }
    return res.dump();
}

void LZUNetwork::set_account(const std::string& account) noexcept {
    this->_account = account;
    auto p = account.find('@');
    this->_username = account.substr(0, p);
}
void LZUNetwork::set_password(const std::string& password) noexcept {
    this->_password = base64_encode(URIComponentEncode(password));
}