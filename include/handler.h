#pragma once

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

class Handler {
public:
    http::response<http::string_body>
    handle(const http::request<http::string_body>& req);

private:
    http::response<http::string_body> handle_register(const http::request<http::string_body>& req);
};