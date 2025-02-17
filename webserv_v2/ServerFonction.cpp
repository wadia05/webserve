#include "webserve.hpp"

int server::getPort() const  {
    return port;
}

std::string server::getServerIP() const  {
    return serverIp;
}