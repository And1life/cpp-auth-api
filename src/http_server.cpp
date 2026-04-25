#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include "http_server.h"
#include <spdlog/spdlog.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

HttpServer::HttpServer(int port) : port_(port), io_context_(1), db_(), handler_(db_)
{
}

void HttpServer::run()
{
    if (!db_.connect()) {
        spdlog::critical("DB connection failed. Server will not start");
        return;
    }

    try
    { 
        spdlog::info("Server started on port {}", port_);
        accept_loop();
    }
    catch(const std::exception& e)
    {
        spdlog::critical("Server error: {}", e.what());
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
        spdlog::debug("Handling new session");

        beast::flat_buffer buffer;

        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        spdlog::debug("Request received");

        http::response<http::string_body> res = handler_.handle(req);

        http::write(socket, res);

        spdlog::debug("Response sent");

        socket.shutdown(tcp::socket::shutdown_send);
    }
    catch(const std::exception& e)
    {
        spdlog::error("Session error: {}", e.what());
    }
}
