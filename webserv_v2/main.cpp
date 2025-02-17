#include "webserve.hpp"
static int nbClients = 0;
void server::setupServer()
{
    int opt = 1;
    this->fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd_server == -1)
        throw std::runtime_error("Socket creation failed");

    if(setsockopt(this->fd_server, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) < 0)   
        throw std::runtime_error("Setsockopt failed");
    
    this->server_addr.sin_family = AF_INET;
    // this->server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // this->server_addr.sin_addr.s_addr = inet_addr(serverIp.c_str());
    this->server_addr.sin_addr.s_addr = INADDR_ANY;  
    // std::cout << "test  >>  " << inet_addr(serverIp.c_str()) << std::endl; 
    this->server_addr.sin_port = htons(this->port);

    if(bind(this->fd_server, (sockaddr*)&this->server_addr, sizeof(this->server_addr)) < 0)
        throw std::runtime_error("Bind failed");
    if(listen(this->fd_server, 50) < 0)
        throw std::runtime_error("Listen failed");
    std::cout << this->serverName  << " is run on "<< "http:" << "//" << this->serverIp << ":" << this->port << std::endl;    
}
int sender(client &Client , std::vector<client> &clients)
{
    if (Client.method == GET)
    {
        std::string body = "<!DOCTYPE html>\n"
           "<html lang=\"en\">\n"
           "<head>\n"
           "    <meta charset=\"UTF-8\">\n"
           "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
           "    <title>File Upload</title>\n"
           "</head>\n"
           "<body>\n"
           "    <h2>Upload a File</h2>\n"
           "    <form action=\"http://192.168.3.31:8080\" method=\"POST\" enctype=\"multipart/form-data\">\n"
           "        <input type=\"file\" name=\"file\">\n"
           "        <button type=\"submit\">Upload</button>\n"
           "    </form>\n"
           "    <form action=\"http://192.168.3.31:8080\" method=\"DELETE\">\n"
           "        <button type=\"submit\">Delete</button>\n"
           "    </form>\n"
           "</body>\n"
           "</html>";

        std::string success_message = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.length()) + "\r\n\r\n" +body;
        send(Client.fd_client, success_message.c_str(), success_message.length(), 0);
        
        std::cout << "Client disconnected. Total clients: " << nbClients << "\n";
        nbClients--;
        close(Client.fd_client);
        clients.erase(clients.begin() + Client.vIndex);
    }
    else
    {
        std::string body = "<html><body><h1>Method Not Supported</h1></body></html>";
        std::string success_message = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.length()) + "\r\n\r\n" +body;
        send(Client.fd_client, success_message.c_str(), success_message.length(), 0);
        
        std::cout << "Client disconnected. Total clients: " << nbClients << "\n";
        nbClients--;
        close(Client.fd_client);
        clients.erase(clients.begin() + Client.vIndex);
    }
    return 0;
}
int  handler(client &client , char *buffer)
{
    if (client.method == GET)
        std::cout << "\033[1;32mGET request detected\033[0m\n";
    else if (client.method == POST)
        std::cout << "\033[1;32mPOST request detected\033[0m\n";
    else if (client.method == DELETE)
        std::cout << "\033[1;32mDELETE request detected\033[0m\n";
    if (std::string(buffer).find("boundary") != std::string::npos)
    {
        std::cout << "boundary detected\n";
        if (std::string(buffer).length() < BUFFER_SIZE)
            client.finish = true;
        else
        {
            client.bodyFond= true;
            // return 0;
        }
        return 1;
    }
    else  if (client.bodyFond == false && std::string(buffer).find("\r\n\r\n") != std::string::npos)
    {
        std::cout << "File end detected\n";
        client.finish = true;
        return 0;
    }
    return 1;
}

// void setMethod(client &client, char *buffer)
// {   

// }

void server::run_server()
{
    std::vector<client> clients;
    while (true)
    {
        clients.push_back(client());
        socklen_t client_len = sizeof(clients.back().client_address);
        int new_fd = accept(this->fd_server, (sockaddr*)&clients.back().client_address, &client_len);

        if (new_fd < 0)
        {
            std::cerr << "Error accepting client: " << strerror(errno) << std::endl;
            clients.pop_back();
            continue;
        }
        clients.back().fd_client = new_fd;
        nbClients++;

        char buffer[BUFFER_SIZE] = {0};
        int received;

        for (size_t i = 0; i < clients.size(); )
        {
            received = recv(clients[i].fd_client, buffer, BUFFER_SIZE, 0);
            std::cout <<  buffer << std::endl;
            if (received <= 0)
            {
                close(clients[i].fd_client);
                clients.erase(clients.begin() + i);
                nbClients--;
                continue;
            }

            clients[i].fileSizeRead += received;
            clients[i].vIndex = i;
            if (std::string(buffer).find("GET") != std::string::npos)
                clients[i].method = GET;
            else if (std::string(buffer).find("POST") != std::string::npos)
                clients[i].method = POST;
            else if (std::string(buffer).find("DELETE") != std::string::npos)
                clients[i].method = DELETE;
            if (handler(clients[i], buffer) == 0)
            {
                sender(clients[i], clients);
            }
            
            if (clients.size() - 1 == i) 
                i = 0;
            else 
                i++; 
        }
    }
}


server::server()
{
    this->port = 8080;
    this->max_upload_size = 25000000; // approximately 25 MB
    this->serverName = "MoleServer";
    this->serverIp = "192.168.3.31";
    this->root = "webserv_v2/root/moleServer";
    this->index_page = "webserv_v2/root/moleServer/index.html";
    this->error_page = "webserv_v2/root/moleServer/404.html";
    setupServer();
}

server::~server()
{
    close(this->fd_server);
    // for (size_t i = 0; i < client.size(); i++) {
    //     close(clients[i].fd_client);
    // }
}


int main ()
{
    try
    {
        server s1;
        s1.run_server();

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

