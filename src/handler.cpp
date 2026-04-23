#include "handler.h"
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

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


    std::string username = data["username"];
    std::string email = data["email"];
    std::string phone = data["phone"];
    std::string password = data["password"];

    std::cout << "Parsed user: " << username << ", " << email << std::endl;
    
    http::response<http::string_body> res {
        http::status::ok,
        req.version()
    };

    res.set(http::field::content_type, "application/json");

    json response = {
        {"success", true},
        {"message", "JSON parsed successfully"},
        {"user", {
            {"username", username},
            {"email", email}
        }}
    };

    res.body() = response.dump();
    res.prepare_payload();

    return res;
}
