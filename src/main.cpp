#include "encoder.h"
#include "lzunet.h"
#include <nlohmann/json_fwd.hpp>
#include <format>
#include <fstream>
#include <iostream>
#include <CLI/CLI.hpp>
#include <utils.h>

void output_status(const std::string& status_info);
void output_login(const std::string& login_info);
void output_logout(const std::string& logout_info);

int main(int argc, char *argv[]) {
    initMappings();

    CLI::App app("Network CLI");

    // params
    std::string username;
    std::string password;

    // add config
    std::string config_file = get_user_home() + "lzunc_config.yaml";
    app.set_config("--config", config_file, "read a yaml file");
    
    // add command 
    auto config = app.add_subcommand("config", "<configs>");
    auto config_username = config->add_subcommand("username", "username for school net");
    auto config_password = config->add_subcommand("password", "password for school net");
    config_username->add_option("value", username)->required();
    config_password->add_option("value", password)->required();

    auto login = app.add_subcommand("login", "login");
    auto logout = app.add_subcommand("logout", "logout");
    auto status = app.add_subcommand("status", "check status");

    CLI11_PARSE(app, argc, argv);

    LZUNetwork lzunet(username, password);

    if (*config) {
        lzunet.set_username(username);
        lzunet.set_password(password);

        std::ofstream out(config_file);
        out << app.config_to_str(false, false);
        out.close();
    } else if (*login) {
        std::string login_info = lzunet.login();
        output_login(login_info);
    } else if (*logout) {
        std::string logout_info = lzunet.logout();
        output_logout(logout_info);
    } else if (*status) {
        std::string status_info = lzunet.status();
        output_status(status_info);
    }

    return 0;
}

void output_status(const std::string& status_info) {
    try {
        auto j = nlohmann::json::parse(status_info);
        if (j["error"] == "ok") {
            std::cout << std::format("{:<20}\t{:<15}\t{:<15}\t{:<15}\t{:<15}\n",
                        "Username", "IP", "Sumbytes", "RemainBytes", "Products");
            std::cout << std::format("{:<20}\t{:<15}\t{:<15}\t{:<15}\t{:<15}\n",
                        j["user_name"].get<std::string>(),
                        j["online_ip"].get<std::string>(),
                        std::format("{:.2f}", j["sum_bytes"].get<double>() / (1024 * 1024)) + "MB",
                        std::format("{:.2f}", j["remain_bytes"].get<double>() / (1024 * 1024)) + "MB",
                        j["products_name"].get<std::string>());
        } else if (j["error"] == "not_online_error") {
            std::cout << "Not Online" << "\n";
        } else {
            std::cout << "Unknown Error" << "\n";
            std::cout << status_info << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "parse error: " << e.what() << "\n";
    }
} 

void output_login(const std::string& login_info) {
    try {
        auto j = nlohmann::json::parse(login_info);
        if (j["error"] == "ok") {
            std::cout << "Login Successful" << "\n";
            std::cout << std::format("Username: {}\n", j["username"].get<std::string>());
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