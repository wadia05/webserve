#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
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
    this->server_addr.sin_addr.s_addr = INADDR_ANY;  
    this->server_addr.sin_port = htons(this->port);

    if(bind(this->fd_server, (sockaddr*)&this->server_addr, sizeof(this->server_addr)) < 0)
        throw std::runtime_error("Bind failed");
    if(listen(this->fd_server, 50) < 0)
        throw std::runtime_error("Listen failed");
    std::cout << this->serverName  << " is run on "<< "http:" << "//" << this->serverIp << ":" << this->port << std::endl;    
}

void cleanup_client(client &Client, std::vector<client> &clients) {
    std::cout << "Client disconnected. Total clients: " << nbClients - 1 << std::endl;
    nbClients--;
    close(Client.fd_file);
    clients.erase(clients.begin() + Client.vIndex);
}

int sender(client &Client, std::vector<client> &clients) {
    if (Client.method == GET) {
        // If file couldn't be opened, send error response
        if (!Client.file || !Client.file->is_open()) {
            std::string errorResponse = prepareErrorResponse(404);
            send(Client.fd_client, errorResponse.c_str(), errorResponse.length(), 0);
            Client.finish = true;
            cleanup_client(Client, clients);
            return 0;
        }
        
        // Calculate total file size if not done yet
        if (Client.fullfileSize == 0 && !Client.hedersend) {
            Client.file->seekg(0, std::ios::end);
            Client.fullfileSize = Client.file->tellg();
            Client.file->seekg(0, std::ios::beg);
            std::cout << "File size: " << Client.fullfileSize << " bytes" << std::endl;
        }
        
        // Send headers first if not sent yet
        if (!Client.hedersend) {
            std::string headers = prepareResponseHeaders(Client);
            std::cout << headers << std::endl;
            send(Client.fd_client, headers.c_str(), headers.length(), 0);
            Client.hedersend = true;
            std::cout << "Headers sent" << std::endl;
        }
        
        // Send a chunk of the file body
        memset(Client.buffer, 0, BUFFER_SIZE);
        Client.file->read(Client.buffer, BUFFER_SIZE);
        std::streamsize bytesRead = Client.file->gcount();
        
        if (bytesRead > 0) {
            // Send the data chunk without headers
            int bytesSent = send(Client.fd_client, Client.buffer, bytesRead, 0);
            if (bytesSent < 0) {
                std::cerr << "Error sending data: " << strerror(errno) << std::endl;
                Client.finish = true;
                cleanup_client(Client, clients);
            } else {
                std::cout << "Sent " << bytesSent << " bytes of data" << std::endl;
            }
            return 0; // Continue with next iteration
        } else {
            // End of file reached
            std::cout << "End of file reached" << std::endl;
            Client.finish = true;
            cleanup_client(Client, clients);
            return 0;
        }
    }
    return 1;
}

// Helper function to cleanup client connection

int server::handler(client &client)
{
    if (client.method == GET)
    {
        std::cout << "\033[1;32mGET request detected\033[0m\n";
        if (GET_hander(client, client.buffer) == 0)
            return 0;
        return 1;
    }
    else if (client.method == POST)
    {
        std::cout << "\033[1;32mPOST request detected\033[0m\n";
        client.finish = true;
        return 0;
    }
    else if (client.method == DELETE)
    {
        client.finish = true;
        std::cout << "\033[1;32mDELETE request detected\033[0m\n";
    }
    
    return 1;
}

void reader(client &client)
{
    int received = recv(client.fd_client, client.buffer, BUFFER_SIZE, 0);
    if (received == 0)
    {
        std::cout << "Client disconnected" << std::endl;
    }
    else            
    {
        std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
    }
    // client.finish = true;
    return;
    
    // client.fileSizeRead += received;
}

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

        for (size_t i = 0; i < clients.size(); )
        {
            if (clients[i].hedersend == false)
                reader(clients[i]);
            std::cout << "---\n";
            clients[i].vIndex = i;
            if (std::string(clients[i].buffer).find("GET") != std::string::npos)
                clients[i].method = GET;
            else if (std::string(clients[i].buffer).find("POST") != std::string::npos)
                clients[i].method = POST;
            else if (std::string(clients[i].buffer).find("DELETE") != std::string::npos)
                clients[i].method = DELETE;

            std::cout << clients[i].buffer << std::endl;
            std::cout << "cilent  "<< i << std::endl;
            handler(clients[i]);
            sender(clients[i] , clients);

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
    this->root = "../webserv_v2/root";
    this->index_page = "../webserv_v2/root/index.html";
    this->error_page = "../webserv_v2/root/moleServer/404.html";
    setupServer();
}

server::~server()
{
    close(this->fd_server);
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
