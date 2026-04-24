#pragma once

#include <string>
#include <optional>

struct User
{
    std::string id;
    std::string username;
    std::string email;
    std::string phone;
    std::string password;

    std::string role = "individual";

    std::optional<std::string> company_name;
    std::optional<std::string> inn;
    std::optional<std::string> metadata;
};
