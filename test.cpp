#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <sstream>

#define MAX_EVENTS 50
#define BUFFER_SIZE 4096

struct TestClient {
    int socket;
    std::string request;
    bool headersSent;
    bool requestComplete;
    
    TestClient(int sock) : socket(sock), headersSent(false), requestComplete(false) {}
};

class TestMultiplexServer {
private:
    std::vector<int> serverSockets;
    std::vector<std::shared_ptr<TestClient>> clients;
    int epollFd;
    struct epoll_event events[MAX_EVENTS];

    void setNonBlocking(int sock) {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

    bool sendResponse(std::shared_ptr<TestClient> client) {
        if (!client->headersSent) {
            std::string response = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "Content-Length: 13\r\n"
                                 "Connection: close\r\n"
                                 "\r\n"
                                 "Hello World!\n";
            
            ssize_t sent = send(client->socket, response.c_str(), response.length(), MSG_NOSIGNAL);
            if (sent <= 0) {
                return false;
            }
            client->headersSent = true;
            return true;
        }
        return false;
    }

    void handleClientData(std::shared_ptr<TestClient> client) {
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;

        while ((bytesRead = recv(client->socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytesRead] = '\0';
            client->request.append(buffer);

            // Check if we've received the end of the HTTP request
            if (client->request.find("\r\n\r\n") != std::string::npos) {
                client->requestComplete = true;
                
                // Modify epoll to watch for write events
                struct epoll_event ev;
                ev.events = EPOLLOUT | EPOLLET;
                ev.data.fd = client->socket;
                epoll_ctl(epollFd, EPOLL_CTL_MOD, client->socket, &ev);
                break;
            }
        }

        if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN)) {
            // Connection closed or error
            closeClient(client);
        }
    }

    void closeClient(std::shared_ptr<TestClient> client) {
        epoll_ctl(epollFd, EPOLL_CTL_DEL, client->socket, nullptr);
        close(client->socket);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [client](const std::shared_ptr<TestClient>& c) {
                return c->socket == client->socket;
            }), clients.end());
        std::cout << "Client disconnected. Remaining clients: " << clients.size() << "\n";
    }

    void handleNewClient(int serverSocket) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        while (true) {  // Accept all pending connections
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // No more pending connections
                }
                perror("Accept failed");
                break;
            }
            std::cout << "New connection from " << clientSocket << std::endl;
            setNonBlocking(clientSocket);

            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;  // Start with reading the request
            ev.data.fd = clientSocket;
            
            if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &ev) == -1) {
                perror("Failed to add client to epoll");
                close(clientSocket);
                continue;
            }

            clients.push_back(std::make_shared<TestClient>(clientSocket));
            std::cout << "New client connected. Total clients: " << clients.size() << "\n";
        }
    }

public:
    TestMultiplexServer() {
        epollFd = epoll_create1(0);
        if (epollFd == -1) {
            perror("Epoll create failed");
        }
    }

    bool addServer(int port) {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Failed to create socket");
            return false;
        }

        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("Bind failed");
            close(serverSocket);
            return false;
        }

        if (listen(serverSocket, SOMAXCONN) < 0) {
            perror("Listen failed");
            close(serverSocket);
            return false;
        }

        setNonBlocking(serverSocket);

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = serverSocket;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &ev) == -1) {
            perror("Epoll control failed");
            close(serverSocket);
            return false;
        }

        serverSockets.push_back(serverSocket);
        std::cout << "Test server started on port " << port << "\n";
        return true;
    }

    void run() {
        std::cout << "Test multiplexing server is running...\n";
        while (true) {
            int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                perror("Epoll wait failed");
                break;
            }

            for (int i = 0; i < nfds; i++) {
                int currentFd = events[i].data.fd;

                // Check if it's a server socket
                bool isServerSocket = false;
                for (int sock : serverSockets) {
                    if (currentFd == sock) {
                        isServerSocket = true;
                        handleNewClient(currentFd);
                        break;
                    }
                }

                if (!isServerSocket) {
                    auto clientIt = std::find_if(clients.begin(), clients.end(),
                        [currentFd](const std::shared_ptr<TestClient>& client) {
                            return client->socket == currentFd;
                        });

                    if (clientIt != clients.end()) {
                        auto client = *clientIt;
                        
                        if (events[i].events & EPOLLIN) {
                            handleClientData(client);
                        }
                        
                        if (events[i].events & EPOLLOUT && client->requestComplete) {
                            if (!sendResponse(client)) {
                                closeClient(client);
                            } else if (client->headersSent) {
                                // Response sent completely, close connection
                                closeClient(client);
                            }
                        }
                    }
                }
            }
        }
    }

    ~TestMultiplexServer() {
        for (int sock : serverSockets) {
            close(sock);
        }
        for (const auto& client : clients) {
            close(client->socket);
        }
        close(epollFd);
    }
};

int main() {
    TestMultiplexServer server;
    server.addServer(8080);
    server.run();
    return 0;
}