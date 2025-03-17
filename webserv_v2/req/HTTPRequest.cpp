#include "HTTPRequest.hpp"

bool HTTPRequest::parse_request(const std::string &request)
{
    std::istringstream iss(request);
    std::string line;
    if (!std::getline(iss, line) || !parseRequestLine(line))
        return false;
    while (std::getline(iss, line) && !line.empty() && line != "\r")
    {
        trim(line);
        if (!parseHeaderLine(line))
            return false;
    }
    if (method == "POST")
    {
        if (!parseBody(iss))
            return false;
    }
    // print_all();
    return true;
}