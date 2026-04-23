#include "validator.h"

#include <regex>
#include <set>

ValidationResult Validator::validate_register(const User& user)
{
    if (user.username.empty() || user.username.size() > 64 )
    {
        return {false, "Invalid username length"};
    }
    
}