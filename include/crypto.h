#pragma once

#include <string>

class PasswordHasher {
public:
    static std::string hash(const std::string& password);
};