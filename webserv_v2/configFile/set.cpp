#include "Config.hpp"

void Config::setListen(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_error("listen", i);
        return;
    }
    std::string host = "0.0.0.0";
    std::string port;
    std::string &value = tokens[0].value;
    size_t pos = value.find(':');
    if (pos != std::string::npos)
    {
        host = value.substr(0, pos);
        port = value.substr(pos + 1);
        if (host.empty() || port.empty() || !isValidIPAddress(host) || !isValidPort(port))
        {
            print_error("listen", i);
            return;
        }
    }
    else
    {
        port = value;
        if (port.empty() || !isValidPort(port))
        {
            print_error("listen", i);
            return;
        }
    }
    std::map<std::string, std::string> listeen;
    listeen[host] = port;
    this->listen.push_back(listeen);
}

void Config::setServerName(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_error("server_name", i);
        return;
    }
    server_name.push_back(tokens[0].value);
}

void Config::setErrorPage(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 2)
    {
        print_error("error_page", i);
        return;
    }
    std::istringstream s(tokens[0].value + " " + tokens[1].value);
    std::string path;
    int code;
    s >> code >> path;
    if (s.fail() || !s.eof() || path.empty() || (code < 400 || code > 599) || !isValidPath(path, false))
    {
        print_error("error_page", i);
        return;
    }
    std::map<int, std::string> error_pagee;
    error_pagee[code] = tokens[1].value;
    error_page.push_back(error_pagee);
}

void Config::setClientMaxBodySize(std::vector<t_token> &tokens, int *x)
{
    if (tokens.size() != 1)
    {
        print_error("client_max_body_size", x);
        return;
    }
    std::string &value = tokens[0].value;
    size_t len = value.size();
    char c = std::tolower(value[len - 1]);
    if (!std::isalpha(c) && !std::isdigit(c))
    {
        print_error("client_max_body_size", x);
        return;
    }
    if (std::isdigit(c))
        c = '\0';
    std::string num = value.substr(0, len - (std::isalpha(c) ? 1 : 0));
    for (size_t i = 0; i < num.size(); i++)
    {
        if (!isdigit(num[i]))
        {
            print_error("client_max_body_size", x);
            return;
        }
    }
    long size = atol(num.c_str());
    if (size < 0 || (c != 'k' && c != 'm' && c != 'g' && c != '\0'))
    {
        print_error("client_max_body_size", x);
        return;
    }
    if (c == 'k')
        size *= 1024;
    else if (c == 'm')
        size *= 1024 * 1024;
    else if (c == 'g')
        size *= 1024 * 1024 * 1024;
    client_max_body_size.push_back(size);
}

void Config::Location::setAutoindex(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_error("autoindex", i);
        return;
    }
    if (tokens[0].value != "on" && tokens[0].value != "off")
    {
        print_error("autoindex", i);
        return;
    }
    autoindex.push_back(tokens[0].value);
}

void Config::Location::setAllowMethods(std::vector<t_token> &tokens, int *x)
{
    if (tokens.size() < 1 || tokens.size() > 3)
    {
        print_error("allow_methods", x);
        return;
    }
    int methods[3] = {0, 0, 0};
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].value == "GET" && methods[1] + methods[2] == 0 && methods[0] == 0)
            methods[0] = 1;
        else if (tokens[i].value == "POST" && (methods[0] == 0 || methods[0] == 1) && methods[2] == 0 && methods[1] == 0)
            methods[1] = 1;
        else if (tokens[i].value == "DELETE" && ((methods[0] == 0 || methods[0] == 1) && (methods[1] == 0 || methods[1] == 1)) && methods[2] == 0)
            methods[2] = 1;
        else
        {
            print_error("allow_methods", x);
            return;
        }
    }
    std::string value;
    if (methods[0] == 1)
        value += "GET";
    if (methods[1] == 1)
        value += (value.empty() ? "POST" : " POST");
    if (methods[2] == 1)
        value += (value.empty() ? "DELETE" : " DELETE");
    allow_methods.push_back(value);
}

void Config::Location::setReturn(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 2)
    {
        print_error("return", i);
        return;
    }
    int code;
    std::string path;
    std::istringstream s(tokens[0].value + " " + tokens[1].value);
    s >> code >> path;
    if (s.fail() || !s.eof() || path.empty())
    {
        print_error("return", i);
        return;
    }
    if (code < 300 || code > 399)
    {
        print_error("return", i);
        return;
    }
    if (path.substr(0, 7) != "http://" && path.substr(0, 8) != "https://")
    {
        print_error("return", i);
        return;
    }
    std::map<int, std::string> return__;
    return__[code] = tokens[1].value;
    return_.push_back(return__);
}

void Config::Location::setRoot(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_error("root", i);
        return;
    }
    std::string &path = tokens[0].value;
    if (!isValidPath(path, true))
    {
        print_error("root", i);
        return;
    }
    root.push_back(path);
}

void Config::Location::setIndex(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_error("index", i);
        return;
    }
    std::string &file = tokens[0].value;
    if (file.empty() || !isValidPath(file, false))
    {
        print_error("index", i);
        return;
    }
    index.push_back(file);
}

void Config::Location::setUploadDir(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 1)
    {
        print_error("upload_dir", i);
        return;
    }
    std::string &path = tokens[0].value;
    if (!isValidPath(path, true))
    {
        print_error("upload_dir", i);
        return;
    }
    if (access(path.c_str(), W_OK) != 0)
    {
        print_error("upload_dir", i);
        return;
    }
    upload_dir.push_back(path);
}

void Config::Location::setCgi(std::vector<t_token> &tokens, int *i)
{
    if (tokens.size() != 2)
    {
        print_error("cgi", i);
        return;
    }
    std::string &extension = tokens[0].value;
    std::string &path = tokens[1].value;
    if (extension.empty() || extension[0] != '.' || extension.length() < 2)
    {
        print_error("cgi", i);
        return;
    }
    if (!isValidPath(path, false))
    {
        print_error("cgi", i);
        return;
    }
    if (access(path.c_str(), X_OK) != 0)
    {
        print_error("cgi", i);
        return;
    }
    std::map<std::string, std::string> cgii;
    cgii[extension] = path;
    cgi.push_back(cgii);
}

void Config::Location::setPath(std::string path, int *i)
{
    if (path.empty() || path[0] != '/' || path[path.size() - 1] != '/')
    {
        print_error("after location block", i);
        return;
    }
    this->path = path;
}