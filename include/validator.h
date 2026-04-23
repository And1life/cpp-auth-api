#pragma once

#include <string>
#include "user.h"

struct ValidationResult
{
    bool ok;
    std::string message;
};

class Validator
{
public:
    static ValidationResult validate_register(const User& user);
};
