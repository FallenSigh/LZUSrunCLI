#pragma once

#include <cpr/cpr.h>
#include <string>

class LZUNetwork {
public:
    LZUNetwork(const std::string& username, const std::string& password);
    std::string get_challenge();
    std::string login();
    std::string logout();
    std::string status();
    void set_username(const std::string& username) noexcept;
    void set_password(const std::string& password) noexcept;

private:
    std::string _username;
    std::string _password;
    std::string _ip;
};