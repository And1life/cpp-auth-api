#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include "http_server.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

HttpServer::HttpServer(int port) : port_(port), io_context_(1)
{
}

void HttpServer::run()
{
    try
    {
        std::cout << "Server started on port " << port_ << std::endl;
        accept_loop();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << '\n';
    }
}

void HttpServer::accept_loop()
{
    tcp::acceptor acceptor{io_context_, {tcp::v4(), static_cast<unsigned short>(port_)}};

    while (true)
    {
        tcp::socket socket(io_context_);
        acceptor.accept(socket);
        
        std::thread( &HttpServer::handle_session, this, std::move(socket)).detach();
    }
}

void HttpServer::handle_session(tcp::socket socket)
{
    try
    {
        beast::flat_buffer buffer;

        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        std::cout << "Incoming request: "
                  << req.method_string() << " "
                  << req.target() << std::endl;

        http::response<http::string_body> res (http::status::ok, req.version());

        res.set(http::field::content_type, "application/json");
        res.body() = R"({"message": "Server is running"})";
        res.prepare_payload();

        http::write(socket, res);
        socket.shutdown(tcp::socket::shutdown_send);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Session error: " << e.what() << '\n';
    }
}
