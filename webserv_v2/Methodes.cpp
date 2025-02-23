#include "webserve.hpp"

void server::extractGET(client &client, char *buffer)
{
    
    std::string str = buffer;
    std::string path;
    size_t pos = str.find("GET ");
    size_t pos1 = str.find(" HTTP/1.1");
    if (pos != std::string::npos && pos1 != std::string::npos)
    {
        path = str.substr(pos + 4, pos1 - pos - 4);
        if (path == "/")
            path = index();
        else
            path = getRoot() + path;
        client.filePath = path;
        std::cout << path << std::endl;
        client.fd_file = open(path.c_str(), O_RDONLY);
        if (client.fd_file == -1)
        {
            std::cerr << "Error opening file: " << strerror(errno) << std::endl;
            return;
        }
        client.bodyFond = true;
    }
    else
    {
        std::cerr << "Error extracting path" << std::endl;
        return;
    }

}
int server::GET_hander(client &client, char *buffer)
{
    
    // std::cout << client.fd_client << std::endl;
    // std::cout << client.fd_file<< std::endl;
    // std::cout << "extractGET" << std::endl;  
    if(client.bodyFond == false)
    {
        extractGET(client, buffer);
        int swich = client.fd_client;
        client.fd_client = client.fd_file;
        client.fd_file = swich;
        return 0;
    }

   

        // std::cout << "extractGETmed" << std::endl;
    
    // std::cout << "extractGETend" << std::endl;
    // read and send file
    return 1;
}

// void POST_handler(client &client, char *buffer)
// {

// }

// void DELETE_handler(client &client, char *buffer)
// {

// }