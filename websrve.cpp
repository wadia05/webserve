#include "websrve.hpp"


void error(std::string str)
{
    write(2, str.c_str(), str.length());
    write(2, "\n", 1); // Add a newline
}
int setupServer()
{
    int opt = 1;
    int fd_server = socket(AF_INET, SOCK_STREAM, 0);// create a socket
    if (fd_server == -1){
        error("Socket creation failed");
        exit (EXIT_FAILURE);
    }
    //setsockeopt is help to avoid a binding error when the \
    OS keeps the port reserved for a while (TIME_WAIT state). \
    and help to opetimze the socket
    if(setsockopt(fd_server,SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) < 0)
    {
        error("Setsockopt failed");
        exit (EXIT_FAILURE);
    }

    //binding
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; //Uses IPv4 for the socket.
    server_addr.sin_addr.s_addr = INADDR_ANY; //Binds to all available network interfaces.
    //This ensures compatibility across different CPU architectures.
    server_addr.sin_port = htons(PORT);//Converts and sets port 
    //attaching socket to the port
    if(bind(fd_server, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        error("bind failed");
        exit (EXIT_FAILURE);
    }
    // Listen for incoming connections
    if (listen(fd_server ,MAX_CONN) < 0)
    {
        error("listen failed");
        close(fd_server);
        exit (EXIT_FAILURE);
    }
    return fd_server;
}
void print_vector(std::vector<std::string> &v)
{
    int i  = 0;
    while (i < v.size())
    {
        std::cout << v[i] << std::endl;
        i++;
    }
    std::cout << std::endl;
}
std::vector<std::string> read_all(int fd_client)
{
    char buffer[BUFFER_SIZE] = {0};
    std::vector<std::string> request;
    ssize_t readed;
    std::string full_request;

    while ((readed = read(fd_client, buffer, BUFFER_SIZE)) > 0)
    {
        full_request.append(buffer, readed);
        if (full_request.find("\r\n\r\n") != std::string::npos) // End of HTTP request
            break;
    }

    if (readed == 0) {
        std::cout << "Client disconnected" << std::endl;
    } else if (readed < 0) {
        error("Read error");
    }

    // Manually split the string into lines
    size_t pos = 0;
    while ((pos = full_request.find("\r\n")) != std::string::npos)
    {
        std::string line = full_request.substr(0, pos);
        request.push_back(line);
        full_request.erase(0, pos + 2); // Remove the processed line (2 chars for "\r\n")
    }
    return request;
}



void runServer(int fd_server)
{
    while (true)
    {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int fd_client_new = accept(fd_server , (sockaddr*)&client_address, &client_len);
        if(fd_client_new < 0)
        {   error("accrept cliean fail");
            continue;
        }
        std::vector<std::string> request = read_all(fd_client_new);
        print_vector(request);

        //temprary solution
        //should handel Get requist
        std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Length: 5\r\n"
                       "Connection: close\r\n"
                       "Content-Type: text/plain\r\n\r\n"
                       "Hello";
        send(fd_client_new, response.c_str(), response.length(), 0);
        close(fd_client_new);
    }
}
int main ()
{
    int fd_server = setupServer();
    std::cout << "Server started on port " << PORT << std::endl;
    runServer(fd_server);
    close(fd_server);
}