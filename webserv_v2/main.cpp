
#include "webserv.hpp"
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
    this->server_addr.sin_addr.s_addr = inet_addr(this->serverIp.c_str());  
    this->server_addr.sin_port = htons(this->port);

    if(bind(this->fd_server, (sockaddr*)&this->server_addr, sizeof(this->server_addr)) < 0)
        throw std::runtime_error("Bind failed");
    if(listen(this->fd_server, 50) < 0)
        throw std::runtime_error("Listen failed");
    std::cout << this->serverName  << " is run on "<< "http:" << "//" << this->serverIp << ":" << this->port << std::endl;    
}

void cleanup_client(client &Client, std::vector<client> &clients) {
    std::cout << "\033[1;33mClient disconnected. Total clients: " << nbClients - 1 << "\033[0m" << std::endl;
    nbClients--;
    
    // Make sure to close the file if it's open
    if (Client.file && Client.file->is_open()) {
        Client.file->close();
        delete Client.file;
        // Client.file = nullptr;
    }
    
    // Close the file descriptor if it's valid
    if (Client.fd_file > 0) {
        close(Client.fd_file);
        Client.fd_file = -1;
    }
    
    // Close the client socket if it's valid
    if (Client.fd_client > 0) {
        close(Client.fd_client);
        Client.fd_client = -1;
    }
    
    // Remove from clients vector if index is valid
    if (Client.vIndex >= 0 && Client.vIndex < static_cast<int>(clients.size())) {
        clients.erase(clients.begin() + Client.vIndex);
    }
}

int PrepareDataToSend(client &Client, int epollfd, struct epoll_event &ev) {
    if (Client.method == GET) {
        // If file couldn't be opened, send error response
        if (!Client.file || !Client.file->is_open()) {
            Client.G_P_Responce = prepareErrorResponse(404);
            epoll_ctl(epollfd, EPOLL_CTL_DEL, Client.fd_client, &ev);
            Client.finish = true;
            return 0;
        }
        
        // Calculate total file size if not done yet
        if (Client.fullfileSize == 0 && !Client.hedersend) {
            Client.file->seekg(0, std::ios::end);
            Client.fullfileSize = Client.file->tellg();
            Client.file->seekg(0, std::ios::beg);
            std::cout << "\033[1;36mFile size: " << Client.fullfileSize << " bytes\033[0m" << std::endl;
        }
        
        // Send headers first if not sent yet
        if (!Client.hedersend) {
            Client.G_P_Responce = prepareResponseHeaders(Client);
            Client.hedersend = true;
            std::cout << "\033[1;32mHeaders prepared\033[0m" << std::endl;
            return 0; // Send headers in the first call to sender
        }
        
        // Send a chunk of the file body
        memset(Client.buffer, 0, BUFFER_SIZE);
        Client.file->read(Client.buffer, BUFFER_SIZE);
        std::streamsize bytesRead = Client.file->gcount();
        Client.fileSizeRead += bytesRead;
        
        if (bytesRead > 0) {
            // Create a proper binary string from the buffer data
            Client.G_P_Responce = std::string(Client.buffer, bytesRead);
            return 0; // Continue with next iteration
        }
        else if (Client.fileSizeRead == Client.fullfileSize) {
            std::cout << "\033[1;31mFile reading complete\033[0m" << std::endl;
            Client.G_P_Responce = ""; // No more data to send
            epoll_ctl(epollfd, EPOLL_CTL_DEL, Client.fd_client, &ev);
            Client.finish = true;
            return 0;
        }
    }
    else if (Client.method == POST)
    {
        // std::cout << Client. << std::endl;
        // std::cout << "post request is complete" << std::endl;
        Client.G_P_Responce = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: 200\r\n"
                             "\r\n"
                             "<html><body>"
                             "<h1>Upload Successful!</h1>"
                             "<p>Your file was successfully uploaded to the server.</p>"
                             "<p><a href='/'>Return to homepage</a></p>"
                             "</body></html>";
        Client.hedersend = true;
        return 0;
    }
    return 1;
}

// Helper function to cleanup client connection

int server::handler(client &client)
{
    // if (client.bodyFond == false)
    // {
    //     client.G_P_Responce.append(client.buffer);
    //     return 1;
    // }
    HTTPRequest request;
    if (!request.parse_request(client.G_P_Responce))
    {
        std::cerr << "Failed to parse request" << std::endl;
        client.filePath = this->error_page;
        return 1;
    }
    request.print_all();
    if (client.method == NOTDETECTED)
    {
        std::string buffer_str(client.G_P_Responce);
        if(buffer_str.find("GET") != std::string::npos)
            client.method = GET;
        else if(buffer_str.find("POST") != std::string::npos)
            client.method = POST;
        else if(buffer_str.find("DELETE") != std::string::npos)
            client.method = DELETE;
            
    }
    if (client.method == GET)
    {
        std::cout << "\033[1;32mGET request detected\033[0m\n";
        if (GET_hander(client) == 0)
            return 0;
        return 1;
    }
    else if (client.method == POST)
    {
        std::cout << "\033[1;32mPOST request detected\033[0m\n";
        if (POST_handler(client) == 0)
            return 0;
        return 1;
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
    // std::cout << client.buffer << std::endl;
    if (received > 0 ) {
       
        client.buffer[received] = '\0'; // Ensure null-termination for string operations
        client.G_P_Responce.append(client.buffer);
        if (received < BUFFER_SIZE)
            client.bodyFond = true;
        // Data received successfully
        std::cout << "\033[1;32mReceived " << received << " bytes\033[0m" << std::endl;
        return received;
    } else if (received == 0) {
        // Client closed connection
        client.bodyFond = true;
        std::cout << "\033[1;33mClient disconnected\033[0m" << std::endl;
        return 0;
    } else {
        // Error occurred
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Non-blocking socket and no data available
            return 1; // Try again later
        } else {
            std::cerr << "\033[1;31mError receiving data: " << strerror(errno) << "\033[0m" << std::endl;
            return -1;
        }
    }
}
int sender(client &Client, std::vector<client> &clients) {
    if (Client.finish == true) {
        cleanup_client(Client, clients);
        return 0;
    }
    
    int sent = send(Client.fd_client, Client.G_P_Responce.c_str(), Client.G_P_Responce.length(), 0);
    if (sent == 0)
    {
        std::cerr << "\033[1;33mClient disconnected "<< Client.fd_client << "\033[0m" << std::endl;
        Client.finish = true;
        return 0;
    }
    if (sent < 0) {
        std::cerr << "\033[1;31mError sending data: " << strerror(errno) << "\033[0m" << std::endl;
        Client.finish = true;
        return 0;
    }
    
    // Clear the response after sending to free memory
    Client.G_P_Responce.clear();
    return 1;
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
        
        for (int j = 0; j < rfds; j++)
        {
            if (events[j].data.fd == this->fd_server)
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
                std::cout << "\033[1;32mNew client connected\033[0m" << std::endl;  // Green color
                } else {
        // Find which client matches this event
                bool clientFound = false;
                for (size_t i = 0; i < clients.size(); i++) {
                    if (events[j].data.fd == clients[i].fd_client) {
                        clientFound = true;
                        // Store the current index for use in cleanup
                        clients[i].vIndex = i;
                        
                        if (events[j].events & EPOLLIN) {
                            std::cout << "\033[1;34mClient " << clients[i].fd_client << " starting read\033[0m" << std::endl;
                            int readResult = reader(clients[i]);
                            if (readResult >= 0 && clients[i].bodyFond == true) {
                                handler(clients[i]);
                                std::cout << "handler is complete" << std::endl;
                                ev.events = EPOLLOUT;
                                ev.data.fd = clients[i].fd_client;
                                epoll_ctl(epollfd, EPOLL_CTL_MOD, clients[i].fd_client, &ev);
                                // std::cout << clients[i] .G_P_Responce << std::endl;
                            }
                        } else if (events[j].events & EPOLLOUT) {
                            std::cout << "\033[1;35mClient " << clients[i].fd_client << " starting write\033[0m" << std::endl;
                            if (PrepareDataToSend(clients[i], epollfd, ev) == 0) {
                                std::cout << "\033[1;32mClient " << clients[i].fd_client << " starting send\033[0m" << std::endl;
                                sender(clients[i], clients);
                            }
                        }
                        break;
            }
        }
        
        if (!clientFound) {
            // Event for a client that's no longer in our list
            std::cout << "\033[1;31mEvent for unknown client, removing from epoll\033[0m" << std::endl;
            epoll_ctl(epollfd, EPOLL_CTL_DEL, events[j].data.fd, &ev);

        }
            // std::cout << "events size " << events.size() << std::endl;

            // for (size_t i = 0; i < clients.size(); i++)
            // {
            //     std::cout << "clients size " << clients.size() << std::endl;
            //     if (events[i].data.fd == clients[i].fd_client)
            //     {
            //         if (events[i].events & EPOLLIN)
            //         {
            //             std::cout << "\033[1;34mClient start read\033[0m" << std::endl;  // Blue color
            //             reader(clients[i]);
            //             if (handler(clients[i]) == 0)
            //             {
            //                 ev.events = EPOLLOUT;
            //                 ev.data.fd = clients[i].fd_client;
            //                 epoll_ctl(epollfd, EPOLL_CTL_MOD, clients[i].fd_client, &ev);
            //             }
            //         }
            //         else if (events[i].events & EPOLLOUT)
            //         {
            //             std::cout << "\033[1;35mClient start write\033[0m" << std::endl;  // Magenta color
            //             if (PrepareDataToSend(clients[i], epollfd, ev) == 0)
            //             {
            //                 std::cout << "\033[1;32mClient start send\033[0m" << std::endl;  // Green color
            //                 if (sender(clients[i], clients) == 0)
            //                 {
            //                     std::cout << "\033[1;33mClient send complete\033[0m" << std::endl;  // Yellow color
            //                 }
            //             }
            //         }
            //     }
            // }
            // else 
            // {
            //     // Handle client read/write here
            // }
        }
        }
    }
    // Clean up
    close(epollfd);
}


server::server(const Config &config)
{

    std::vector<std::map<std::string, std::string> > listenn = config.getListen();
    this->serverIp = listenn[0].begin()->first;
    this->port = std::atoi(listenn[0].begin()->second.c_str());
    this->max_upload_size = config.getClientMaxBodySize()[0];
    this->serverName = config.getServerName()[0];
    this->root = "../webserv_v2/www";
    this->index_page = "../webserv_v2/www/index.html";
    this->error_page = "../webserv_v2/www/error_pages/404.html";
    setupServer();
}

server::~server()
{
    close(this->fd_server);
}
void initMimeTypes(const std::string &filePath, std::map<std::string, std::string> &mimeTypes)
{
    std::ifstream file(filePath.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error opening the MIME types file" << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream linestream(line);
        std::string extension;
        std::string mimeType;
        std::getline(linestream, extension, ',');
        std::getline(linestream, mimeType);
        mimeTypes[extension] = mimeType;
    }
    file.close();
}
int main (int ac, char **av)
{
    if (ac != 2)
        return (std::cerr << "Invalid number of arguments" << std::endl, 1);
    std::map<std::string, std::string> mimeTypes;
    initMimeTypes("www/mimeTypes.csv", mimeTypes);
    std::ifstream file(av[1]);
    Config config;
    config.parser(file);
    std::vector<Config> configs = config.getConfigs();
    try
    {
        server s1(configs[0]);

        s1.run_server();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}