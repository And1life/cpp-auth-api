#include "handler.h"
#include <iostream>
#include "json.hpp"
#include "user.h"
#include "validator.h"
#include "db.h"
#include "crypto.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;

Handler::Handler(Database& db) : db_(db)
{

}

http::response<http::string_body> Handler::handle(const http::request<http::string_body> &req)
{
    spdlog::info("Incoming request: method={}, target={}",
             std::string(req.method_string()),
             std::string(req.target()));

    if (req.target() == "/auth/register")
    {
        spdlog::debug("Routing to /auth/register");
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
        spdlog::warn("Invalid method for /auth/register: {}",
             std::string(req.method_string()));

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

    spdlog::debug("Parsing JSON body");

    try
    {
        data = json::parse(req.body());
    }
    catch(...)
    {
        spdlog::warn("Invalid JSON received");

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

    spdlog::debug("Checking required fields");

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

        spdlog::warn("Missing required fields");
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
            spdlog::warn("Invalid metadata format (not object)");

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
    
    spdlog::debug("Validating user data");

    auto result = Validator::validate_register(user);

    if (!result.ok)
    {
        spdlog::warn("Validation failed: {}", result.message);

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
    
    spdlog::debug("Checking if user exists: email={}, username={}",
             user.email, user.username);

    auto existing = db_.find_user(user.email, user.username);

    if (existing)
    {
        if (existing->email == user.email &&
            existing->username == user.username)
        {
            spdlog::info("User already exists (idempotent): {}", user.email);

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

        spdlog::warn("User already exists: {}", user.email);

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

    spdlog::debug("Hashing password");

    std::string password_hash = PasswordHasher::hash(user.password);
    user.password.clear();

    spdlog::debug("Inserting user into DB: {}", user.email);

    auto id = db_.insert_user(user, password_hash);

    if (!id)
    {
        spdlog::error("Database insert failed for {}", user.email);

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
    

    spdlog::info("User registered successfully: id={}, email={}", *id, user.email);

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
