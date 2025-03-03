#include "webserve.hpp"

std::string prepareResponseHeaders(client &Client) {
    std::string contentType;
    
    // Determine content type based on file extension
    std::string extension;
    size_t dotPos = Client.filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = Client.filePath.substr(dotPos + 1);
    }
    
    // Set content type based on extension
    if (extension == "html" || extension == "htm") {
        contentType = "text/html";
    } else if (extension == "css") {
        contentType = "text/css";
    } else if (extension == "js") {
        contentType = "application/javascript";
    } else if (extension == "jpg" || extension == "jpeg") {
        contentType = "image/jpeg";
    } else if (extension == "png") {
        contentType = "image/png";
    } else if (extension == "gif") {
        contentType = "image/gif";
    } else if (extension == "svg") {
        contentType = "image/svg+xml";
    } else if (extension == "json") {
        contentType = "application/json";
    } else if (extension == "pdf") {
        contentType = "application/pdf";
    } else if (extension == "txt") {
        contentType = "text/plain";
    } else {
        contentType = "application/octet-stream";
    }
    
    // Build HTTP response header
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << Client.fullfileSize << "\r\n";
    ss << "Server: MoleServer\r\n";
    ss << "Connection: keep-alive\r\n";
    ss << "\r\n"; // Empty line to separate headers from body
    
    return ss.str();
}

// Helper function to create error responses
std::string prepareErrorResponse(int statusCode) {
    std::stringstream ss;
    std::string errorBody;
    
    switch (statusCode) {
        case 404:
            ss << "HTTP/1.1 404 Not Found\r\n";
            errorBody = "<html><body><h1>404 Not Found</h1><p>The requested resource could not be found.</p></body></html>";
            break;
        case 403:
            ss << "HTTP/1.1 403 Forbidden\r\n";
            errorBody = "<html><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
            break;
        case 500:
        default:
            ss << "HTTP/1.1 500 Internal Server Error\r\n";
            errorBody = "<html><body><h1>500 Internal Server Error</h1><p>The server encountered an unexpected condition.</p></body></html>";
            break;
    }
    
    ss << "Content-Type: text/html\r\n";
    ss << "Content-Length: " << errorBody.length() << "\r\n";
    ss << "Server: MoleServer\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << errorBody;
    
    return ss.str();
}