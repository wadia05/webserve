#include "websrve.hpp"

void error(std::string str)
{
    write(2, str.c_str(), str.length());
    write(2, "\n", 1);
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
    for (const auto& line : v)
    {
        std::cout << line << std::endl;
    }
    std::cout << std::endl;
}

std::vector<std::string> read_all(int fd_client) {
    char buffer[BUFFER_SIZE] = {0};
    std::vector<std::string> request;
    ssize_t readed;
    std::string full_request;
    size_t content_length = 0;
    bool headers_done = false;

    while ((readed = read(fd_client, buffer, BUFFER_SIZE)) > 0) {
        full_request.append(buffer, readed);
        // Detect end of headers
        // std::cout << buffer ;
        if (!headers_done && full_request.find("\r\n\r\n") != std::string::npos) {
            headers_done = true;

            // Extract headers
            size_t pos = 0;
            while ((pos = full_request.find("\r\n")) != std::string::npos) {
                std::string line = full_request.substr(0, pos);
                request.push_back(line);
                full_request.erase(0, pos + 2);

                // Look for Content-Length
                if (line.find("Content-Length:") != std::string::npos) {
                    content_length = std::stoul(line.substr(16));
                }
            }
        }

        // Read the remaining body if Content-Length is set
        if (headers_done && full_request.length() >= content_length) {
            request.push_back("\r\n\r\n" + full_request); // Store full request
            break;
        }
    }

    if (readed == 0) {
        std::cout << "Client disconnected" << std::endl;
    } else if (readed < 0) {
        error("Read error");
    }

    return request;
}


std::string generate_response()
{
    std::string body ;//= "<!DOCTYPE html>\n"
                    //    "<html lang=\"en\">\n"
                    //    "<head>\n"
                    //    "    <meta charset=\"UTF-8\">\n"
                    //    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                    //    "    <title>File Upload</title>\n"
                    //    "</head>\n"
                    //    "<body>\n"
                    //    "    <h2>Upload a File</h2>\n"
                    //    "    <form action=\"http://localhost:8080\" method=\"POST\" enctype=\"multipart/form-data\">\n"
                    //    "        <input type=\"file\" name=\"file\">\n"
                    //    "        <button type=\"submit\">Upload</button>\n"
                    //    "    </form>\n"
                    //    "    <form action=\"http://localhost:8080\" method=\"DELETE\">\n"
                    //    "        <button type=\"submit\">Delete</button>\n"
                    //    "    </form>\n"
                    //    "</body>\n"
                    //    "</html>";
        std::ifstream file("door.png", std::ios::binary);  // Open in binary mode
        if (!file.is_open()) {
            return "HTTP/1.1 404 Not Found\r\n\r\n";
        }

        // Get file size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read the entire file into a string
        // std::string body;
        body.resize(size);
        file.read(&body[0], size);
        file.close();

        std::ostringstream content_length;
        content_length << body.size();

        std::string response = "HTTP/1.1 200 OK\r\n"
            "Content-Length: " + content_length.str() + "\r\n"
            "Connection: close\r\n"
            "Content-Type: Image/png\r\n\r\n" +
            body;
        return response;
}

void runServer(int fd_server)
{
    while (true)
    {
        char buffer[BUFFER_SIZE] = {0};
        int received;
        int Clenght = 0;
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int fd_client_new = accept(fd_server, (sockaddr*)&client_address, &client_len);
        if (fd_client_new < 0)
        {
            error("Accept client failed");
            continue;
        }

        std::string full_request;
        while((received = recv(fd_client_new, buffer, BUFFER_SIZE, 0)) > 0)
        {
            full_request.append(buffer, received);
            std::cout.write(buffer, received);
            if (full_request.find("\r\n\r\n") != std::string::npos) // End of HTTP request
                break;

            size_t l;
            if((l = full_request.find("content-length")) != std::string::npos)
            {
                std::cout << "-------------------" << std::endl;
                std::cout << l << std::endl;
                Clenght = stoi(full_request.substr(l+16, full_request.find("\r\n", l+16)));
                std::cout << "-------------------" << std::endl;
            }
        }
        // std::cout << full_request ;

        std::string response = generate_response();
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
