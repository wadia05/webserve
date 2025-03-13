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

int sender(client &Client, std::vector<client> &clients, int epollfd, struct epoll_event &ev) {
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
        Client.fileSizeRead += bytesRead;
        
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
        }
        else if (Client.fileSizeRead == Client.fullfileSize) {
            std::cout << "\033[1;31mFile sent successfully\033[0m" << std::endl;
            // End of file reached
            std::cout << "End of file reached" << std::endl;
            epoll_ctl(epollfd, EPOLL_CTL_DEL, Client.fd_client, &ev);
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

int reader(client &client)
{
    int received = recv(client.fd_client, client.buffer, BUFFER_SIZE, 0);
    if (received == 0)
    {
        std::cout << "Client disconnected" << std::endl;
        return 0;
    }
    else            
    {
        std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
        return -1;
    }
    // client.finish = true;
    return 1;
    
    // client.fileSizeRead += received;
}

void setnonblock(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        throw std::runtime_error("fcntl failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl failed");
}

void server::run_server()
{
    int epollfd;
    struct epoll_event ev;
    std::vector<client> clients;
    std::vector<struct epoll_event> events(10); // Initialize with capacity
    
    epollfd = epoll_create(1);
    if (epollfd == -1)
        throw std::runtime_error(std::string("epoll_create failed: ") + strerror(errno));
    
    ev.events = EPOLLIN;
    ev.data.fd = this->fd_server;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->fd_server, &ev) == -1)
        throw std::runtime_error(std::string("epoll_ctl failed: ") + strerror(errno));
    
    setnonblock(this->fd_server);
    
    while (true)
    {
        std::cout << "is waiting for events" << std::endl;
        int rfds = epoll_wait(epollfd, events.data(), events.size(), 1000);
        
        if (rfds == -1) {
            if (errno == EINTR) {
                // Interrupted by a signal, just continue
                continue;
            }
            throw std::runtime_error(std::string("epoll_wait failed: ") + strerror(errno));
        }
        
        // Resize events vector if we're close to capacity
        if (rfds > 0 && static_cast<size_t>(rfds) >= events.size() * 0.8) {
            events.resize(events.size() * 2);
        }
        
        for (int i = 0; i < rfds; i++)
        {
            if (events[i].data.fd == this->fd_server)
            {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                clients.push_back(client());
                clients.back().fd_client = accept(this->fd_server, 
                                                 (struct sockaddr*)&client_addr, 
                                                 &client_len);
                
                if (clients.back().fd_client == -1) {
                    std::cerr << "accept failed: " << strerror(errno) << std::endl;
                    clients.pop_back();
                    continue;
                }
                
                ev.events = EPOLLIN;
                ev.data.fd = clients.back().fd_client;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clients.back().fd_client, &ev) == -1) {
                    std::cerr << "epoll_ctl for client failed: " << strerror(errno) << std::endl;
                    close(clients.back().fd_client);
                    clients.pop_back();
                    continue;
                }
                nbClients++;
                setnonblock(clients.back().fd_client);
                std::cout << "New client connected" << std::endl;
            }
            // std::cout << "events size " << events.size() << std::endl;

            for (size_t i = 0; i < clients.size(); i++)
            {
            std::cout << "clients size " << clients.size() << std::endl;
                if (events[i].data.fd == clients[i].fd_client)
                {
                    if (events[i].events & EPOLLIN)
                    {
                        std::cout << "client start read  " << std::endl;
                        reader(clients[i]);
                        if (handler(clients[i]) == 0)
                        {
                            ev.events = EPOLLOUT;
                            ev.data.fd = clients[i].fd_client;
                            epoll_ctl(epollfd, EPOLL_CTL_MOD, clients[i].fd_client, &ev);
                        }
                        }
                    else if (events[i].events & EPOLLOUT)
                    {
                        std::cout << "client start write " << std::endl;
                        sender(clients[i], clients, epollfd, ev);
                    }
                }
            }
            // else 
            // {
            //     // Handle client read/write here
            // }
        }
    }
    // Clean up
    close(epollfd);
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
