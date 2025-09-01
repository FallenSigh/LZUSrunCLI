#pragma once

#include <cpr/cpr.h>
#include <string>

class LZUNetwork {
public:
    LZUNetwork(const std::string& account, const std::string& password);
    std::string get_challenge();
    std::string login(const std::string& ip = "");
    std::string logout();
    std::string status();
    std::string info();
    void set_account(const std::string& account) noexcept;
    void set_password(const std::string& password) noexcept;

private:
    std::string _username;
    std::string _account;
    std::string _password;
    std::string _encrypted_password;
    std::string _ip;
};