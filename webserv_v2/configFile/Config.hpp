#pragma once
#include "../webserv.hpp"

typedef struct s_token t_token;

class Config
{

public:
    class Location;

private:
    std::vector<Config> configs;
    std::vector<std::string> port;
    std::vector<std::string> host;
    std::map<int, std::string> error_page;
    std::vector<long> client_max_body_size;
    std::vector<std::string> server_name;
    std::vector<Config::Location> locations;

    bool isValidIPAddress(const std::string &ip);
    bool isValidPort(const std::string &port_str);
    static bool isValidPath(const std::string &path, bool isDirectory);
    bool validateserver(Config &tempConfig, int *i);
    bool validatelocation(int *i, Config::Location &tempLocation);

public:
    Config() {};
    void setHost(std::vector<t_token> &tokens, int *i);
    void setPort(std::vector<t_token> &tokens, int *i);
    void setServerName(std::vector<t_token> &tokens, int *i);
    void setErrorPage(std::vector<t_token> &tokens, int *i);
    void setClientMaxBodySize(std::vector<t_token> &tokens, int *i);

    std::vector<std::string> getPort() const;
    std::vector<std::string> getHost() const;
    std::vector<std::string> getServerName() const;
    std::map<int, std::string> getErrorPage() const;
    std::vector<long> getClientMaxBodySize() const;

    std::vector<Config::Location> getLocations() const;
    std::vector<Config> getConfigs() const;
    void addLocation(const Location &location);
    void parser(std::ifstream &file);

    class Location
    {
    private:
        std::string path;
        std::vector<std::string> root;
        std::vector<std::string> upload_dir;
        std::vector<std::string> autoindex;
        std::vector<std::string> index;
        std::vector<std::string> allow_methods;
        std::map<int, std::string> return_;
        std::map<std::string, std::string> cgi;

    public:
        Location() {};
        void setPath(std::string path, int *i);
        void setRoot(std::vector<t_token> &tokens, int *i);
        void setUploadDir(std::vector<t_token> &tokens, int *i);
        void setAutoindex(std::vector<t_token> &tokens, int *i);
        void setIndex(std::vector<t_token> &tokens, int *i);
        void setAllowMethods(std::vector<t_token> &tokens, int *i);
        void setReturn(std::vector<t_token> &tokens, int *i);
        void setCgi(std::vector<t_token> &tokens, int *i);

        std::string getPath() const;
        std::vector<std::string> getRoot() const;
        std::vector<std::string> getUploadDir() const;
        std::vector<std::string> getAutoindex() const;
        std::vector<std::string> getIndex() const;
        std::vector<std::string> getAllowMethods() const;
        std::map<int, std::string> getReturn() const;
        std::map<std::string, std::string> getCgi() const;
    };
    void printConfig(std::vector<Config> &configs) const;
};