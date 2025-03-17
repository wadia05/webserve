#pragma once
#include "../webserv.hpp"

void trim(std::string &str);
bool isHex(char c);
std::string read_file(std::string path);
std::string urlDecode(const std::string &encoded);

struct BodyPart
{
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> data;
    std::map<std::string, std::string> files;
    std::string content_type;
};

class HTTPRequest
{
private:
    std::string method;
    std::string path;
    std::string http_version;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> headers;
    std::vector<BodyPart> body_parts;
public:
    bool hasHeader(const std::string &name) const;
    std::string getHeader(const std::string &name) const;
    void parseQueryString(const std::string &query_string);
    void parseURLEncodedBody(std::string body);
    void parseFirstLine(const std::string &line);

    bool parse_request(const std::string &request);
    bool parseRequestLine(const std::string &line);
    bool parseHeaderLine(const std::string &line);
    bool parseBody(std::istringstream &iss);

    std::string getMethod() const;
    std::string getPath() const;
    std::string getHttpVersion() const;
    const std::map<std::string, std::string> &getQueryParams() const;
    const std::map<std::string, std::string> &getHeaders() const;
    const std::vector<BodyPart> &getBodyParts() const;
    void print_all();
};
