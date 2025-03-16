#include "webserve.hpp"

void server::extractGET(client &client)
{
    
    std::string str = client.buffer;
    std::string path;
    size_t pos = str.find("GET ");
    size_t pos1 = str.find(" HTTP/1.1");
    if (pos != std::string::npos && pos1 != std::string::npos)
    {
        path = str.substr(pos + 4, pos1 - pos - 4);
        std::cout << "Path: " << path << std::endl;
        if (path == "/")
            path = index();
        else
            path = getRoot() + path;
        client.filePath = path;
        if (client.file == NULL)
        {
            client.file = new std::ifstream();
        }
        client.file->open(path.c_str(), std::ios::in | std::ios::binary);
        if (!client.file->is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return;
        }
        
        // client.bodyFond = true;
    }
    else
    {
        std::cerr << "Error extracting path" << std::endl;
        return;
    }

}
int server::GET_hander(client &client)
{

        extractGET(client);
        return 0;
}

int server::POST_handler(client &client)
{
    client.G_P_Responce.append(client.buffer);
    if (client.bodyFond == true)
    {
        std::cout << client.G_P_Responce << std::endl;
        std::cout << "post request is complete" << std::endl;
        return 0;
    }
    return 1;
}

// void DELETE_handler(client &client, char *buffer)
// {

// }