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
    if(client.bodyFond == false)
    {
        extractGET(client, buffer);
        
        // Calculate total file size for Content-Length header
        if(client.file && client.file->is_open())
        {
            client.file->seekg(0, std::ios::end);
            client.fullfileSize = client.file->tellg();
            client.file->seekg(0, std::ios::beg);
            std::cout << "File size calculated: " << client.fullfileSize << " bytes" << std::endl;
        }
        return 0;
    }
    return 1;
}

// void POST_handler(client &client, char *buffer)
// {

// }

// void DELETE_handler(client &client, char *buffer)
// {

// }