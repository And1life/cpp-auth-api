#pragma once

#include <string>
#include <optional>
#include "user.h"
#include <libpq-fe.h>

class Database {
public:
    Database();
    ~Database();

    bool connect();

    std::optional<User> find_user(const std::string& email, const std::string& username);
    std::optional<std::string> insert_user(const User& user, const std::string& password_hash);

private:
    std::string conninfo_;
    PGconn* conn_;
};