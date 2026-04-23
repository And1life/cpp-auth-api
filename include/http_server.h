#pragma once 

#include <boost/asio.hpp>
#include "handler.h"

class HttpServer {
public:
    explicit HttpServer(int port);

    void run();

private:
    void accept_loop();
    void handle_session(boost::asio::ip::tcp::socket socket);

private:
    int port_;
    boost::asio::io_context io_context_;
    Handler handler_;
        
};