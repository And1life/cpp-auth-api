#include "validator.h"

#include <regex>
#include <set>

ValidationResult Validator::validate_register(const User& user)
{
    if (user.username.empty() || user.username.size() > 64 )
    {
        return {false, "Invalid username length"};
    }
    
    std::regex email_regex(R"(^[\w\.-]+@[\w\.-]+\.\w+$)");
    if (user.email.empty() || user.email.size() > 255 ||
        !std::regex_match(user.email, email_regex))
    {
        return {false, "Invalid email"};
    }

    
    
}