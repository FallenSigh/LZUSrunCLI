#include "encoder.h"
#include "lzunet.h"
#include <nlohmann/json_fwd.hpp>
#include <fstream>
#include <iostream>
#include <CLI/CLI.hpp>
#include <string>
#include <utils.h>

void output_status(const std::string& status_info);
void output_login(const std::string& login_info);
void output_logout(const std::string& logout_info);
void output_info(const std::string& info_info);

int main(int argc, char *argv[]) {
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
    auto config_account = config->add_subcommand("account", "account for school net");
    auto config_password = config->add_subcommand("password", "password for school net");
    config_account->add_option("value", account)->required();
    config_password->add_option("value", password)->required();

    auto login = app.add_subcommand("login", "login");
    login->add_option("IP", ip);
    auto logout = app.add_subcommand("logout", "logout");
    auto status = app.add_subcommand("status", "check status");
    auto info = app.add_subcommand("info", "check data used info");

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
    } else if (*status) {
        std::string status_info = lzunet.status();
        output_status(status_info);
    } else if (*info) {
        std::string info_info = lzunet.info();
        output_info(info_info);
    }

    return 0;
}

void output_info(const std::string& info_info) {
    auto info = nlohmann::json::parse(info_info);

    std::cout << std::format("{:<15}{:<15}{:<20}{:<15}{:<15}", 
    "ProductName", "UsedTraffic", "UsedTime", "UsedCount", "Balance") << "\n";

    if (info.size() == 0) {
        std::cout << "Failed to Get Info." << "\n";
        return;
    }

    for (auto it = info.begin(); it != info.end(); it++) {
        std::cout << std::format("{:<15}{:<15}{:<20}{:<15}{:<15}", 
        (*it)["product_name"].get<std::string>(),
        (*it)["used_traffic"].get<std::string>(),
        (*it)["used_time"].get<std::string>(),
        (*it)["usage_count"].get<std::string>(),
        (*it)["balance"].get<std::string>()) << "\n";
    }
}

void output_status(const std::string& status_info) {
    auto status = nlohmann::json::parse(status_info);

    std::cout << std::format("{:<10}{:<15}{:<20}{:<20}", "ProductName", "Username",  "IP", "OnlineTime") << "\n";

    if (status.size() == 0) {
        std::cout << "Failed to Get Status." << "\n";
        return;
    }

    for (auto it = status.begin(); it != status.end(); it++) {
        std::cout << std::format("{:<10}{:<15}{:<20}{:<20}", 
        (*it)["product_name"].get<std::string>(),
        (*it)["username"].get<std::string>(), 
        (*it)["ip"].get<std::string>(),
        (*it)["online_time"].get<std::string>()) << "\n";
    }
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