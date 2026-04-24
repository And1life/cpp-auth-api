#include "http_server.h"
#include <cstdlib>
#include <iostream>
#include "dotenv.h"

int main(int argc, char const *argv[])
{
    dotenv::init();
    int port;
    if (const char* env_p = std::getenv("SERVER_PORT"))
    {
        port = std::stoi(env_p);
    }
    
    std::cout << "Starting server..." << std::endl;

    HttpServer server(port);
    server.run();
    
    return 0;
}
