#include <bits/stdc++.h>
#include <iostream>

void print_vector(std::vector<char> &v)
{
    int i  = 0;
    while (i < v.size())
    {
        std::cout << v[i] << " - ";
        i++;
    }
    std::cout << std::endl;
}


int main ()
{
    std::vector<int> v1;
    std::vector<char> v2 = {'s','s','s','d','l'};
    std::vector<int> v3 (5, 9);
    std::string str = "hello";
    std::vector<char> v4(str.begin(), str.end());

    // v2.pop_back();
    // v2.insert(v2.begin() , 'c');
    v2.erase (std::find (v2.begin(),  v2.end(), 'd'));
    // std::cout << v2.at(33);
    // print_vector(v1);
    print_vector(v4);
    // print_vector(v3);

    return 0;
}




//read with no blocking 
/*
#include <fcntl.h>   // For fcntl()
#include <cerrno>    // For errno
#include <vector>
#include <iostream>
#include <unistd.h>   // For close()

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 5 // Timeout for retrying recv()

void error(const std::string& str) {
    std::cerr << str << std::endl;
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        error("Failed to get socket flags");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        error("Failed to set socket to non-blocking mode");
    }
}

std::vector<std::string> read_all(int fd_client)
{
    char buffer[BUFFER_SIZE] = {0};
    std::vector<std::string> request;
    ssize_t received;
    std::string full_request;
    int attempts = 0;

    set_nonblocking(fd_client); // Set the socket to non-blocking mode

    while (attempts < TIMEOUT_SEC * 10) // Retry for a maximum of TIMEOUT_SEC seconds
    {
        received = recv(fd_client, buffer, BUFFER_SIZE, MSG_DONTWAIT);

        if (received > 0) {
            full_request.append(buffer, received);
            if (full_request.find("\r\n\r\n") != std::string::npos) // End of HTTP request
                break;
        }
        else if (received == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000); // Sleep for 100ms to avoid busy-waiting
                attempts++;
                continue;
            } else {
                error("Recv error");
                break;
            }
        }
    }

    if (attempts >= TIMEOUT_SEC * 10) {
        error("Receive timed out.");
    }

    // Split request into lines
    size_t pos = 0;
    while ((pos = full_request.find("\r\n")) != std::string::npos) {
        std::string line = full_request.substr(0, pos);
        request.push_back(line);
        full_request.erase(0, pos + 2); // Remove "\r\n"
    }
    return request;
}

*/