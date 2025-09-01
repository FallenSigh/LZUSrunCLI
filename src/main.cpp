#include "encoder.h"
#include "lzunet.h"
#include <nlohmann/json_fwd.hpp>
#include <fstream>
#include <iostream>
#include <CLI/CLI.hpp>
#include <string>
#include <utils.h>

#ifdef _WIN32
#include <Windows.h>
#endif

void output_status(const std::string& status_info);
void output_login(const std::string& login_info);
void output_logout(const std::string& logout_info);
void output_info(const std::string& info_info);

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    ::SetConsoleOutputCP(CP_UTF8);
    #endif
    initMappings();

    CLI::App app("Network CLI");

    // params
    std::string account;
    std::string password;
    std::string ip;

    // add config
    std::string config_file = get_user_home() + "/lzunc_config.yaml";
    app.set_config("--config", config_file, "read a yaml file");
    
    // add command 
    auto config = app.add_subcommand("config", "<configs>");
    auto config_account = config->add_subcommand("account", "account for campus network");
    auto config_password = config->add_subcommand("password", "password for campus network");
    config_account->add_option("value", account)->required();
    config_password->add_option("value", password)->required();

    auto login = app.add_subcommand("login", "login <IP>");
    login->add_option("IP", ip);
    auto logout = app.add_subcommand("logout", "logout");
    auto info = app.add_subcommand("info", "check online info");

    CLI11_PARSE(app, argc, argv);

    LZUNetwork lzunet(account, password);

    if (*config) {
        lzunet.set_account(account);
        lzunet.set_password(password);

        std::ofstream out(config_file);
        out << app.config_to_str(false, false);
        out.close();
    } else if (*login) {
        std::string login_info = lzunet.login(ip);
        output_login(login_info);
    } else if (*logout) {
        std::string logout_info = lzunet.logout();
        output_logout(logout_info);
    } 

    return 0;
}

void output_login(const std::string& login_info) {
    try {
        auto j = nlohmann::json::parse(login_info);
        if (j["error"] == "ok") {
            std::cout << "Login Successful" << "\n";
            std::cout << std::format("account: {}\n", j["username"].get<std::string>());
            std::cout << std::format("ip: {}\n", j["online_ip"].get<std::string>());
            std::cout << std::format("Access Token: {}\n", j["access_token"].get<std::string>());
        } else if (j["error"] == "login_error") {
            std::cout << "Login Error" << "\n";
            std::cout << std::format("Error Msg: {}\n", j["error_msg"].get<std::string>()) << "\n";
        } else {
            std::cout << "Unknown Error" << "\n";
            std::cout << login_info << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "parse error: " << e.what() << "\n";
    }
}

void output_logout(const std::string& logout_info) {
    try {
        auto j = nlohmann::json::parse(logout_info);
        if (j["error"] == "ok") {
            std::cout << "Logout Successful" << "\n";
        } else if (j["error"] == "login_error") {
            std::cout << "Logout Error" << "\n";
            std::cout << std::format("Error Msg: {}\n", j["error_msg"].get<std::string>()) << "\n";
        } else {
            std::cout << "Unknown Error" << "\n";
            std::cout << logout_info << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "parse error: " << e.what() << "\n";
    }
}