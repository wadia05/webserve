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
        // Open the file using fstream
        std::ifstream file(Client.filePath.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << Client.filePath << std::endl;
            // Send error response to client before disconnecting
            const char* error_msg = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(Client.fd_file, error_msg, strlen(error_msg), 0);
            cleanup_client(Client, clients);
            return 1;
        }

        // First send HTTP header
        std::string header = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n\r\n";
        if (send(Client.fd_file, header.c_str(), header.length(), 0) == -1) {
            std::cerr << "Failed to send header" << std::endl;
            cleanup_client(Client, clients);
            return 1;
        }

        // Read and send file in chunks
        char buffer[BUFFER_SIZE];
        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
            std::streamsize bytesRead = file.gcount();
            std::streamsize totalSent = 0;
            
            // Keep trying to send until all bytes are sent
            while (totalSent < bytesRead) {
                ssize_t sent = send(Client.fd_file, buffer + totalSent, 
                                  bytesRead - totalSent, 0);
                if (sent == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Socket buffer is full, wait a bit and retry
                        usleep(1000);  // Sleep for 1ms
                        continue;
                    }
                    std::cerr << "Send error: " << strerror(errno) << std::endl;
                    cleanup_client(Client, clients);
                    return 1;
                }
                totalSent += sent;
            }
        }

        cleanup_client(Client, clients);
        return 0;
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
    int received;

    received = recv(client.fd_client, client.buffer, BUFFER_SIZE, 0);
    if (received <= 0)
    {
        close(client.fd_client);
        nbClients--;
        return;
    }
    client.fileSizeRead += received;
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
            reader(clients[i]);
            clients[i].vIndex = i;
            if (std::string(clients[i].buffer).find("GET") != std::string::npos)
                clients[i].method = GET;
            else if (std::string(clients[i].buffer).find("POST") != std::string::npos)
                clients[i].method = POST;
            else if (std::string(clients[i].buffer).find("DELETE") != std::string::npos)
                clients[i].method = DELETE;

            if(handler(clients[i]) == 0)
            {
                if (clients.size() - 1 == i) 
                    i = 0;
                else 
                    i++; 
                continue;
            }
            std::cout << clients[i].buffer << std::endl;
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
    this->root = "../webserv_v2/root/";
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
