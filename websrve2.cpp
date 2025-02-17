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

    std::string readRequest(int client_fd) {
        // static const size_t BUFFER_SIZE = 4096;
        std::string request;
        char buffer[BUFFER_SIZE];
        size_t content_length = 0;
        bool found_header_end = false;
        
        while (true) {
            ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if (bytes_read <= 0) {
                if (bytes_read < 0) {
                    throw std::runtime_error("Error reading from socket");
                }
                break;
            }
            
            request.append(buffer, bytes_read);
            
            if (!found_header_end) {
                size_t header_end = request.find("\r\n\r\n");
                if (header_end != std::string::npos) {
                    found_header_end = true;
                    
                    size_t content_length_pos = request.find("Content-Length:");
                    if (content_length_pos != std::string::npos) {
                        size_t value_start = request.find_first_not_of(" \t", content_length_pos + 15);
                        if (value_start != std::string::npos) {
                            content_length = std::stoul(request.substr(value_start));
                        }
                    }
                    
                    if (content_length == 0 || 
                        request.length() >= (header_end + 4 + content_length)) {
                        break;
                    }
                }
            } else if (request.length() >= 
                      (request.find("\r\n\r\n") + 4 + content_length)) {
                break;
            }
        }
        
        return request;
    }
std::string generate_response()
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
                       "    <form action=\"http://localhost:8080\" method=\"POST\" enctype=\"multipart/form-data\">\n"
                       "        <input type=\"file\" name=\"file\">\n"
                       "        <button type=\"submit\">Upload</button>\n"
                       "    </form>\n"
                       "    <form action=\"http://localhost:8080\" method=\"DELETE\">\n"
                       "        <button type=\"submit\">Delete</button>\n"
                       "    </form>\n"
                       "</body>\n"
                       "</html>";
 

        std::string response = "HTTP/1.1 200 OK\r\n"
            "Content-Length: " + std::to_string(body.length()) + "\r\n"
            "Connection: close\r\n"
            "Content-Type: text/html\r\n\r\n" +
            body;
        return response;
}

void runServer(int fd_server)
{
    while (true)
    {
        // char buffer[BUFFER_SIZE] = {0};
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

        std::string* full_request = new std::string(readRequest(fd_client_new));
        std::cout << *full_request << std::endl;

        std::string response = generate_response();
        send(fd_client_new, response.c_str(), response.length(), 0);
        delete full_request;
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


// std::string read_all_request(int fd_client) {
//     char buffer[BUFFER_SIZE] = {0};
//     std::string full_request;
//     ssize_t bytes_read;
//     size_t content_length = 0;
//     bool found_header_end = false;
    
//     while ((bytes_read = recv(fd_client, buffer, BUFFER_SIZE, 0)) > 0) {
//         full_request.append(buffer, bytes_read);
        
//         // Find the end of headers if we haven't already
//         if (!found_header_end) {
//             size_t header_end = full_request.find("\r\n\r\n");
//             if (header_end != std::string::npos) {
//                 found_header_end = true;
                
//                 // Convert headers to lowercase for case-insensitive search
//                 // std::string headers = full_request.substr(0, header_end);
//                 // std::transform(headers.begin(), headers.end(), headers.begin(), ::tolower);
                
//                 // Look for content-length header
//                 size_t content_length_pos = full_request.find("Content-length:");
//                 if (content_length_pos != std::string::npos) {
//                     size_t value_start = full_request.find_first_not_of(" \t", content_length_pos + 15);
//                     if (value_start != std::string::npos) {
//                         content_length = std::stoul(full_request.substr(value_start));
//                     }
//                 }
                
//                 // If no body expected, we can stop reading
//                 if (content_length == 0) {
//                     break;
//                 }
                
//                 // If we have the full body already, we can stop
//                 if (full_request.length() >= (header_end + 4 + content_length)) {
//                     break;
//                 }
//             }
//         } else if (full_request.length() >= (full_request.find("\r\n\r\n") + 4 + content_length)) {
//             // We've found headers end previously and now have the full body
//             break;
//         }
//     }
    
//     // if (bytes_read < 0) {
//     //     throw std::runtime_error("Error reading from socket");
//     // }
    
//     return full_request;
// }