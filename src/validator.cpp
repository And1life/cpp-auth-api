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

    if (user.phone.empty() || user.phone.size() > 32)
    {
        return{false, "Invalid phone"};
    }

    if (user.password.size() < 8 || user.password.size() > 20)
    {
        return {false, "Password length must be 8-20"};
    }

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;

    for ( char c : user.password)
    {
        if (std::isupper(c)) has_upper = true;
        if (std::islower(c)) has_lower = true;
        if (std::isdigit(c)) has_digit = true;
    }
    
    if (!has_upper || !has_lower || !has_digit)
    {
        return {false, "Password must contain upper, lower and digit"};
    }
    
    std::set<std::string> allowed_roles = {
        "individual",
        "logistics_partner",
        "business_account"
    };

    if (!allowed_roles.count(user.role))
    {
        return {false, "registration_forbidden"};
    }
    
    if (user.role == "business_account")
    {
        if (!user.company_name.has_value() ||
            user.company_name->empty() ||
            user.company_name->size() > 256)
        {
            return {false, "company_name required for business_account"};
        }
    }
    
    if (user.inn.has_value())
    {
        std::regex inn_regex(R"(^\d{10}|\d{12}$)");
        if (!std::regex_match(*user.inn, inn_regex))
        {
            return {false, "Invalid INN"};
        }
    }
    
    return {true, ""};
    
}