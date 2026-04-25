#include "http_server.h"
#include <cstdlib>
#include <iostream>
#include "dotenv.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char const *argv[])
{
    dotenv::init();
    
    auto logger = spdlog::stdout_color_mt("auth");
    spdlog::set_default_logger(logger);

    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    int port = std::stoi(std::getenv("SERVER_PORT"));

    spdlog::info("Starting server...");

    HttpServer server(port);
    server.run();
    
    return 0;
}
