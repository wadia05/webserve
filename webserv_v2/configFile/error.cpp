#include "Config.hpp"

bool Config::validateserver(Config &tempConfig, int *i)
{
    if (tempConfig.getListen().empty() || tempConfig.getServerName().empty() || tempConfig.getErrorPage().empty() || tempConfig.getClientMaxBodySize().empty())
    {
        print_error("missing required key", i);
        return false;
    }
    std::vector<std::map<std::string, std::string> > listen = tempConfig.getListen();
    std::vector<std::string> port_numbers;
    for (std::vector<std::map<std::string, std::string> >::iterator it = listen.begin(); it != listen.end(); ++it)
    {
        for (std::map<std::string, std::string>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
            port_numbers.push_back(it2->second);
    }
    std::sort(port_numbers.begin(), port_numbers.end());
    if (std::adjacent_find(port_numbers.begin(), port_numbers.end()) != port_numbers.end())
    {
        print_error("repeated port number", i);
        return false;
    }
    std::vector<std::string> server_name = tempConfig.getServerName();
    std::sort(server_name.begin(), server_name.end());
    if (std::adjacent_find(server_name.begin(), server_name.end()) != server_name.end())
    {
        print_error("repeated server name", i);
        return false;
    }
    if (server_name.size() != listen.size())
    {
        print_error("server name and listen size mismatch", i);
        return false;
    }
    std::vector<std::map<int, std::string> > error_page = tempConfig.getErrorPage();
    std::vector<int> error_codes;
    for (std::vector<std::map<int, std::string> >::iterator it = error_page.begin(); it != error_page.end(); ++it)
    {
        for (std::map<int, std::string>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
            error_codes.push_back(it2->first);
    }
    std::sort(error_codes.begin(), error_codes.end());
    if (std::adjacent_find(error_codes.begin(), error_codes.end()) != error_codes.end())
    {
        print_error("repeated error code", i);
        return false;
    }
    if (tempConfig.getClientMaxBodySize().size() > 1)
    {
        print_error("client_max_body_size has more than one value", i);
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
    std::vector<std::map<int, std::string> > return_ = tempLocation.getReturn();
    std::vector<int> return_keys;
    for (std::vector<std::map<int, std::string> >::iterator it = return_.begin(); it != return_.end(); ++it)
    {
        for (std::map<int, std::string>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
            return_keys.push_back(it2->first);
    }
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
    std::vector<std::map<std::string, std::string> > cgi = tempLocation.getCgi();
    std::vector<std::string> cgi_keys;
    for (std::vector<std::map<std::string, std::string> >::iterator it = cgi.begin(); it != cgi.end(); ++it)
    {
        for (std::map<std::string, std::string>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
            cgi_keys.push_back(it2->first);
    }
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
        std::vector<std::map<std::string, std::string> > listen = it->getListen();
        if (!listen.empty())
        {
            printf(GREEN "Listen: " RESET);
            for (std::vector<std::map<std::string, std::string> >::iterator it2 = listen.begin(); it2 != listen.end(); ++it2)
                for (std::map<std::string, std::string>::iterator it3 = it2->begin(); it3 != it2->end(); ++it3)
                    printf(YELLOW "%s:%s " RESET, it3->first.c_str(), it3->second.c_str());
            printf("\n");
        }
        std::vector<std::string> server_name = it->getServerName();
        if (!server_name.empty())
        {
            printf(GREEN "Server Name: " RESET);
            for (std::vector<std::string>::iterator it2 = server_name.begin(); it2 != server_name.end(); ++it2)
                printf(YELLOW "%s " RESET, it2->c_str());
            printf("\n");
        }
        std::vector<std::map<int, std::string> > error_page = it->getErrorPage();
        if (!error_page.empty())
        {
            printf(GREEN "Error Page: " RESET);
            for (std::vector<std::map<int, std::string> >::iterator it2 = error_page.begin(); it2 != error_page.end(); ++it2)
                for (std::map<int, std::string>::iterator it3 = it2->begin(); it3 != it2->end(); ++it3)
                    printf(YELLOW "%d:%s " RESET, it3->first, it3->second.c_str());
            printf("\n");
        }
        std::vector<long> client_max_body_size = it->getClientMaxBodySize();
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
            if (!root.empty())
            {
                printf(GREEN "Root: " RESET);
                for (std::vector<std::string>::iterator it3 = root.begin(); it3 != root.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            std::vector<std::string> upload_dir = it2->getUploadDir();
            if (!upload_dir.empty())
            {
                printf(GREEN "Upload Directory: " RESET);
                for (std::vector<std::string>::iterator it3 = upload_dir.begin(); it3 != upload_dir.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            std::vector<std::string> autoindex = it2->getAutoindex();
            if (!autoindex.empty())
            {
                printf(GREEN "Autoindex: " RESET);
                for (std::vector<std::string>::iterator it3 = autoindex.begin(); it3 != autoindex.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            std::vector<std::string> index = it2->getIndex();
            if (!index.empty())
            {
                printf(GREEN "Index Files: " RESET);
                for (std::vector<std::string>::iterator it3 = index.begin(); it3 != index.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            std::vector<std::string> allow_methods = it2->getAllowMethods();
            if (!allow_methods.empty())
            {
                printf(GREEN "Allowed Methods: " RESET);
                for (std::vector<std::string>::iterator it3 = allow_methods.begin(); it3 != allow_methods.end(); ++it3)
                    printf(YELLOW "%s " RESET, it3->c_str());
                printf("\n");
            }
            std::vector<std::map<int, std::string> > return_ = it2->getReturn();
            if (!return_.empty())
            {
                printf(GREEN "Return: " RESET);
                for (std::vector<std::map<int, std::string> >::iterator it3 = return_.begin(); it3 != return_.end(); ++it3)
                    for (std::map<int, std::string>::iterator it4 = it3->begin(); it4 != it3->end(); ++it4)
                        printf(YELLOW "%d:%s " RESET, it4->first, it4->second.c_str());
                printf("\n");
            }
            std::vector<std::map<std::string, std::string> > cgi = it2->getCgi();
            if (!cgi.empty())
            {
                printf(GREEN "CGI Scripts: " RESET);
                for (std::vector<std::map<std::string, std::string> >::iterator it3 = cgi.begin(); it3 != cgi.end(); ++it3)
                    for (std::map<std::string, std::string>::iterator it4 = it3->begin(); it4 != it3->end(); ++it4)
                        printf(YELLOW "%s:%s " RESET, it4->first.c_str(), it4->second.c_str());
                printf("\n");
            }
        }
        printf(BOLD BLUE "\n<------ End of Server Configuration ------>\n\n" RESET);
    }
}