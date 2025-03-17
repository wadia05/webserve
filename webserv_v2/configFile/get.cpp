#include "Config.hpp"

bool Config::isValidIPAddress(const std::string &ip)
{
    if (ip == "localhost")
        return true;
    std::istringstream ss(ip);
    std::string segment;
    int count = 0;
    while (getline(ss, segment, '.'))
    {
        if ((segment.empty() || segment.length() > 3) || (segment.length() > 1 && segment[0] == '0'))
            return false;
        for (std::string::const_iterator it = segment.begin(); it != segment.end(); ++it)
        {
            if (!isdigit(*it))
                return false;
        }
        long value = atol(segment.c_str());
        if (value < 0 || value > 255)
            return false;
        count++;
    }
    return count == 4;
}

bool Config::isValidPort(const std::string &port_str)
{
    if (port_str.empty() || port_str.length() > 5)
        return false;
    for (std::string::const_iterator it = port_str.begin(); it != port_str.end(); ++it)
    {
        if (!isdigit(*it))
            return false;
    }
    long port = atol(port_str.c_str());
    return port >= 1 && port <= 65535;
}

bool Config::isValidPath(const std::string &path, bool isDirectory)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
    if (isDirectory)
    {
        DIR *dir = opendir(path.c_str());
        if (dir)
        {
            closedir(dir);
            return true;
        }
        return false;
    }
    return S_ISREG(path_stat.st_mode);
}

std::vector<std::map<std::string, std::string> > Config::getListen() const { return listen; }
std::vector<std::string> Config::getServerName() const { return server_name; }

std::vector<std::map<int, std::string> > Config::getErrorPage() const { return error_page; }

std::vector<long> Config::getClientMaxBodySize() const { return client_max_body_size; }
std::vector<std::string> Config::Location::getAutoindex() const { return autoindex; }
std::vector<std::string> Config::Location::getAllowMethods() const { return allow_methods; }
std::vector<std::map<std::string, std::string> > Config::Location::getCgi() const { return cgi; }
std::vector<std::string> Config::Location::getRoot() const { return root; }
std::vector<std::string> Config::Location::getIndex() const { return index; }
std::vector<std::string> Config::Location::getUploadDir() const { return upload_dir; }
std::vector<std::map<int, std::string> > Config::Location::getReturn() const { return return_; }
std::string Config::Location::getPath() const { return path; }
void Config::addLocation(const Location &location)
{
    locations.push_back(location);
}
std::vector<Config::Location> Config::getLocations() const { return locations; }
std::vector<Config> Config::getConfigs() const { return configs; }