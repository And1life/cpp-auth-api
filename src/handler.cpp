#include "handler.h"
#include <iostream>
#include "json.hpp"
#include "user.h"
#include "validator.h"
#include "db.h"
#include "crypto.h"

using json = nlohmann::json;

Handler::Handler(Database& db) : db_(db)
{

}

http::response<http::string_body> Handler::handle(const http::request<http::string_body> &req)
{
    std::cout << "Request: "
              << req.method_string() << " "
              << req.target() << std::endl;

    if (req.target() == "/auth/register")
    {
        return handle_register(req);
    }

    http::response<http::string_body> res{
        http::status::not_found,
        req.version()
    };

    res.set(http::field::content_type, "application/json");
    res.body() = R"({"error":"not_found"})";
    res.prepare_payload();

    return res;
}

http::response<http::string_body> Handler::handle_register(const http::request<http::string_body> &req)
{
    if (req.method() != http::verb::post)
    {
        http::response<http::string_body> res{
            http::status::method_not_allowed,
            req.version()
        };

        res.set(http::field::content_type, "application/json");
        res.body() = R"({"error":"method_not_allowed"})";
        res.prepare_payload();

        return res;
    }

    json data;

    try
    {
        data = json::parse(req.body());
    }
    catch(...)
    {
        http::response<http::string_body> res {
            http::status::bad_request,
            req.version()
        };

        res.set(http::field::content_type, "application/json");
        res.body() = R"({
            "success": false,
            "error": "validation_error",
            "message": "Invalid JSON"
        })";

        res.prepare_payload();
        return res;
    }

    if (!data.contains("username") ||
        !data.contains("email") ||
        !data.contains("phone") ||
        !data.contains("password"))
    {
        http::response<http::string_body> res {
            http::status::bad_request,
            req.version()
        };

        res.set(http::field::content_type, "application/json");
        res.body() = R"({
            "success": false,
            "error": "validation_error",
            "message": "Missing required fields"
        })";

        res.prepare_payload();
        return res;
    }


    User user;

    user.username = data["username"];
    user.email = data["email"];
    user.phone = data["phone"];
    user.password = data["password"];

    if (data.contains("role"))
    {
        user.role = data["role"];
    }

    if(data.contains("company_name"))
    {
        user.company_name = data["company_name"];
    }

    if (data.contains("inn"))
    {
        user.inn = data["inn"];
    }
    
    if (data.contains("metadata"))
    {
        if (!data["metadata"].is_object())
        {
            json err = {
            {"success", false},
            {"error", "validation_error"},
            {"message", "metadata must be object"}
        };
        
        http::response<http::string_body> res {
            http::status::bad_request,
            req.version()
        };

        res.set(http::field::content_type, "application/json");
        res.body() = err.dump();
        res.prepare_payload();
        
        return res;
        }
        
        user.metadata = data["metadata"].dump();
    }
    
    auto result = Validator::validate_register(user);

    if (!result.ok)
    {
        http::status status = http::status::bad_request;

        if (result.message == "registration_forbidden")
        {
            status = http::status::forbidden;
        }

        http::response<http::string_body> res {status, req.version()};
        
        json error = {
            {"success", false},
            {"error", result.message == "registration_forbidden"
                ? "registration_forbidden"
                : "validation_error"},
                {"message", result.message}
            };
            
        res.set(http::field::content_type, "application/json");
        res.body() = error.dump();
        res.prepare_payload();
        return res;
    }
    
    
    auto existing = db_.find_user(user.email, user.username);

    if (existing)
    {
        if (existing->email == user.email &&
            existing->username == user.username)
        {
            json response = {
                {"success", true},
                {"message", "User already exists (idempotent)"},
                {"user", {
                    {"id", existing->id},
                    {"username", existing->username},
                    {"email", existing->email},
                    {"role", existing->role}
                }}
            };

            http::response<http::string_body> res {
                http::status::ok,
                req.version()
            };

            res.set(http::field::content_type, "application/json");
            res.body() = response.dump();
            res.prepare_payload();
            return res;
        }

        json err = {
            {"success", false},
            {"error", "registration_failed"},
            {"message", "User already exists"}
        };
        
        http::response<http::string_body> res {
            http::status::bad_request,
            req.version()
        };

        res.set(http::field::content_type, "application/json");
        res.body() = err.dump();
        res.prepare_payload();
        return res;
    }

    std::string password_hash = PasswordHasher::hash(user.password);
    user.password.clear();

    auto id = db_.insert_user(user, password_hash);

    if (!id)
    {
        json err = {
            {"success", false},
            {"error", "internal_error"},
            {"message", "Database insert failed"}
        };

        http::response<http::string_body> res {
            http::status::internal_server_error,
            req.version()
        };

        res.set(http::field::content_type, "application/json");
        res.body() = err.dump();
        res.prepare_payload();
        return res;
    }
    
    

    std::cout << "Parsed user: " << user.username << ", " << user.email << std::endl;

    json response = {
        {"success", true},
        {"message", "User registered successfully."},
        {"user", {
            {"id", *id},
            {"username", user.username},
            {"email", user.email},
            {"role", user.role}
        }}
    };
    
    http::response<http::string_body> res {
        http::status::ok,
        req.version()
    };

    res.set(http::field::content_type, "application/json");
    res.body() = response.dump();
    res.prepare_payload();

    return res;
}
