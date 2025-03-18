#include "webserv.hpp"


int server::GET_hander(client &client, HTTPRequest &request)
{
    if(request.getPath() == "/")
        client.filePath = index();
    else
        client.filePath = getRoot()+ "/" + request.getPath();
    if (client.file == NULL)
    {
        client.file = new std::ifstream();
    }
    client.file->open(client.filePath.c_str(), std::ios::in | std::ios::binary);
    if (!client.file->is_open())
    {
        std::cerr << "Failed to open file: " << client.filePath << std::endl;
        client.filePath = error();
        client.file->open(client.filePath.c_str(), std::ios::in | std::ios::binary);
        if (!client.file->is_open())
        {
            std::cerr << "Failed to open file: " << client.filePath << std::endl;
            return 0;
        }
        return 0;
    }
    // extractGET(client);
    return 0;
}

int server::POST_handler(client &client, HTTPRequest &request)
{
   
    // client.G_P_Responce.append(client.buffer);
    if (client.bodyFond == true)
    {
        std::string content_type = request.getHeader("content-type");
        // content_type.
        std::cout << YELLOW << content_type << RESET << std::endl;
        // std::cout << client.G_P_Responce << std::endl;
        //<----------  TO-DO  ---------->//
       /*
        if contebt type is multipart 
            find folder should be save in
            find boundary
            find content langth
            splait the body with boundary
            splait the part2  with \r\n\r\n
                in paert1 find file name
                if content langth is not equal to part2 size
                    return error
                else
                    save the part2 in file
        else if 
       */



        // std::vector<std::string> part = split(client.G_P_Responce, "\r\n\r\n");
        // printer(part);

        std::cout << "post request is complete" << std::endl;
        return 0;
    }
    return 1;
}


std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    // Add the last token
    tokens.push_back(str.substr(start));

    return tokens;
}

int printer(std::vector<std::string> parts) {
    for (size_t i = 0; i < parts.size(); ++i) {
        std::cout << "Part " << i + 1 << ": " << parts[i] << std::endl;
    }

    return 0;
}
// void DELETE_handler(client &client, char *buffer)
// {

// }