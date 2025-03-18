#include "Config.hpp"

bool Config::validateserver(Config &tempConfig, int *i)
{
    if (tempConfig.getHost().empty() || tempConfig.getPort().empty() || tempConfig.getServerName().empty() || tempConfig.getErrorPage().empty() || tempConfig.getClientMaxBodySize().empty())
    {
        print_error("missing required key", i);
        return false;
    }
    if (tempConfig.getHost().size() != 1)
    {
        print_error("host has more than one value", i);
        return false;
    }
    if (tempConfig.getServerName().size() != 1)
    {
        print_error("server_name has more than one value", i);
        return false;
    }
    if (tempConfig.getClientMaxBodySize().size() != 1)
    {
        print_error("client_max_body_size has more than one value", i);
        return false;
    }
    std::vector<std::string> port_numbers = tempConfig.getPort();
    std::sort(port_numbers.begin(), port_numbers.end());
    if (std::adjacent_find(port_numbers.begin(), port_numbers.end()) != port_numbers.end())
    {
        print_error("repeated port number", i);
        return false;
    }
    std::map<int, std::string> error_page = tempConfig.getErrorPage();
    std::vector<int> error_codes;
    for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it)
        error_codes.push_back(it->first);
    std::sort(error_codes.begin(), error_codes.end());
    if (std::adjacent_find(error_codes.begin(), error_codes.end()) != error_codes.end())
    {
        print_error("repeated error code", i);
        return false;
    }
    return true;
}

bool Config::validatelocation(int *i, Config::Location &tempLocation)
{
    if (tempLocation.getAllowMethods().empty() && tempLocation.getAutoindex().empty() && tempLocation.getCgi().empty() && tempLocation.getIndex().empty() && tempLocation.getReturn().empty() && tempLocation.getRoot().empty() && tempLocation.getUploadDir().empty())
    {
        print_error("location block is empty", i);
        return false;
    }
    if (tempLocation.getAutoindex().size() > 1)
    {
        print_error("autoindex has more than one value", i);
        return false;
    }
    std::vector<std::string> index = tempLocation.getIndex();
    std::sort(index.begin(), index.end());
    if (std::adjacent_find(index.begin(), index.end()) != index.end())
    {
        print_error("repeated index value", i);
        return false;
    }
    if (tempLocation.getRoot().size() > 1)
    {
        print_error("root has more than one value", i);
        return false;
    }
    std::map<int, std::string> return_ = tempLocation.getReturn();
    std::vector<int> return_keys;
    for (std::map<int, std::string>::iterator it = return_.begin(); it != return_.end(); ++it)
        return_keys.push_back(it->first);
    std::sort(return_keys.begin(), return_keys.end());
    if (std::adjacent_find(return_keys.begin(), return_keys.end()) != return_keys.end())
    {
        print_error("repeated return key", i);
        return false;
    }
    if (tempLocation.getUploadDir().size() > 1)
    {
        print_error("upload_dir has more than one value", i);
        return false;
    }
    std::map<std::string, std::string> cgi = tempLocation.getCgi();
    std::vector<std::string> cgi_keys;
    for (std::map<std::string, std::string>::iterator it = cgi.begin(); it != cgi.end(); ++it)
        cgi_keys.push_back(it->first);
    std::sort(cgi_keys.begin(), cgi_keys.end());
    if (std::adjacent_find(cgi_keys.begin(), cgi_keys.end()) != cgi_keys.end())
    {
        print_error("repeated cgi key", i);
        return false;
    }
    return true;
}

void Config::printConfig(std::vector<Config> &configs) const
{
    for (std::vector<Config>::iterator it = configs.begin(); it != configs.end(); ++it)
    {
        printf(BOLD BLUE "\n<------ Server Configuration ------>\n" RESET);
        std::vector<std::string> host = it->getHost();
        std::vector<std::string> port = it->getPort();
        std::vector<std::string> server_name = it->getServerName();
        std::map<int, std::string> error_page = it->getErrorPage();
        std::vector<long> client_max_body_size = it->getClientMaxBodySize();
        if (!host.empty())
        {
            printf(GREEN "Host: " RESET);
            for (std::vector<std::string>::iterator it2 = host.begin(); it2 != host.end(); ++it2)
                printf(YELLOW "%s " RESET, it2->c_str());
            printf("\n");
        }
        if (!port.empty())
        {
            printf(GREEN "Port: " RESET);
            for (std::vector<std::string>::iterator it2 = port.begin(); it2 != port.end(); ++it2)
                printf(YELLOW "%s " RESET, it2->c_str());
            printf("\n");
        }
        if (!server_name.empty())
        {
            printf(GREEN "Server Name: " RESET);
            for (std::vector<std::string>::iterator it2 = server_name.begin(); it2 != server_name.end(); ++it2)
                printf(YELLOW "%s " RESET, it2->c_str());
            printf("\n");
        }
        if (!error_page.empty())
        {
            printf(GREEN "Error Page: " RESET);
            for (std::map<int, std::string>::iterator it2 = error_page.begin(); it2 != error_page.end(); ++it2)
                printf(YELLOW "%d:%s " RESET, it2->first, it2->second.c_str());
            printf("\n");
        }
        if (!client_max_body_size.empty())
        {
            printf(GREEN "Client Max Body Size: " RESET);
            for (std::vector<long>::iterator it2 = client_max_body_size.begin(); it2 != client_max_body_size.end(); ++it2)
                printf(YELLOW "%ld " RESET, *it2);
            printf("\n");
        }
        std::vector<Config::Location> locations = it->getLocations();
        for (std::vector<Config::Location>::iterator it2 = locations.begin(); it2 != locations.end(); ++it2)
        {
            printf(BOLD CYAN "\n[Location: %s]\n" RESET, it2->getPath().c_str());
            std::vector<std::string> root = it2->getRoot();
            std::vector<std::string> upload_dir = it2->getUploadDir();
            std::vector<std::string> autoindex = it2->getAutoindex();
            std::vector<std::string> index = it2->getIndex();
            std::vector<std::string> allow_methods = it2->getAllowMethods();
            std::map<int, std::string> return_ = it2->getReturn();
            std::map<std::string, std::string> cgi = it2->getCgi();
            if (!root.empty())
            {
                printf(GREEN "Root: " RESET);
                for (std::vector<std::string>::iterator it3 = root.begin(); it3 != root.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            if (!upload_dir.empty())
            {
                printf(GREEN "Upload Directory: " RESET);
                for (std::vector<std::string>::iterator it3 = upload_dir.begin(); it3 != upload_dir.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            if (!autoindex.empty())
            {
                printf(GREEN "Autoindex: " RESET);
                for (std::vector<std::string>::iterator it3 = autoindex.begin(); it3 != autoindex.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            if (!index.empty())
            {
                printf(GREEN "Index Files: " RESET);
                for (std::vector<std::string>::iterator it3 = index.begin(); it3 != index.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            if (!allow_methods.empty())
            {
                printf(GREEN "Allowed Methods: " RESET);
                for (std::vector<std::string>::iterator it3 = allow_methods.begin(); it3 != allow_methods.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            if (!return_.empty())
            {
                printf(GREEN "Return: " RESET);
                for (std::map<int, std::string>::iterator it3 = return_.begin(); it3 != return_.end(); ++it3)
                    printf(YELLOW "%d:%s " RESET, it3->first, it3->second.c_str());
                printf("\n");
            }
            if (!cgi.empty())
            {
                printf(GREEN "CGI Scripts: " RESET);
                for (std::map<std::string, std::string>::iterator it3 = cgi.begin(); it3 != cgi.end(); ++it3)
                        printf(YELLOW "%s:%s " RESET, it3->first.c_str(), it3->second.c_str());
                printf("\n");
            }
        }
    }
}