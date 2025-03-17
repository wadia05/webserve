#include "HTTPRequest.hpp"

void HTTPRequest::parseQueryString(const std::string &query_string)
{
    std::istringstream iss(query_string);
    std::string key_value;
    while (std::getline(iss, key_value, '&'))
    {
        trim(key_value);
        size_t pos = key_value.find('=');
        if (pos != std::string::npos)
        {
            std::string key = key_value.substr(0, pos);
            std::string value = key_value.substr(pos + 1);
            trim(key);
            trim(value);
            query_params[urlDecode(key)] = urlDecode(value);
        }
        else
            query_params[urlDecode(key_value)] = "";
    }
}

bool HTTPRequest::parseRequestLine(const std::string &line)
{
    std::istringstream iss(line);
    if (!(iss >> method >> path >> http_version))
        return (std::cerr << RED << "Invalid request: " << line << RESET << std::endl, false);
    trim(method);
    trim(path);
    trim(http_version);
    if (method != "GET" && method != "POST" && method != "DELETE")
        return (std::cerr << RED << "Invalid request method: " << method << RESET << std::endl, false);
    if (http_version != "HTTP/1.1")
        return (std::cerr << RED << "Invalid HTTP version: " << http_version << RESET << std::endl, false);
    size_t pos = path.find("?");
    if (pos != std::string::npos)
    {
        parseQueryString(path.substr(pos + 1));
        path = path.substr(0, pos);
    }
    path = urlDecode(path);
    if (path.empty() || path[0] != '/')
        return (std::cerr << RED << "Invalid path: " << path << RESET << std::endl, false);
    if (path == "/")
        return true;
    if (path[0] == '/')
        path = path.substr(1);
    return true;
}
