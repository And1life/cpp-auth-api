#include "handler.h"
#include <iostream>


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

    std::string body = req.body();

    std::cout << "Register body: " << body << std::endl;

    http::response<http::string_body> res {
        http::status::ok,
        req.version()
    };

    res.set(http::field::content_type, "application/json");
    res.body() = R"({
        "success": true,
        "message": "register endpoint works"
    })";

    res.prepare_payload();
    return res; 
}
