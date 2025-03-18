#include "Config.hpp"

void Config::parser(std::ifstream &file)
{
    Tokenizer tokenizer = Tokenizer(file);
    tokenizer.processFile();
    this->configs = std::vector<Config>();
    Config tempConfig;
    Config::Location tempLocation;
    std::vector<std::string> lines = tokenizer.getLines();
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
    {
        std::vector<t_token> tokens = tokenizer.tokenize(*it);
        if (tokens.empty())
            continue;
        int i = 0;
        int x = 0;
        int z = 0;
        for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
        {
            if (it->type == KEY)
            {
                std::vector<t_token> new_tokens;
                for (std::vector<t_token>::iterator it2 = it + 1; it2 != tokens.end() && it2->type == VALUE; ++it2)
                    new_tokens.push_back(*it2);
                if (it->value == "host")
                    tempConfig.setHost(new_tokens, &i);
                else if (it->value == "port")
                    tempConfig.setPort(new_tokens, &i);
                else if (it->value == "server_name")
                    tempConfig.setServerName(new_tokens, &i);
                else if (it->value == "error_page")
                    tempConfig.setErrorPage(new_tokens, &i);
                else if (it->value == "client_max_body_size")
                    tempConfig.setClientMaxBodySize(new_tokens, &i);
                else
                    print_error("unknown key", &i);
                if (i == 1)
                    break;
            }
            else if (it->type == LOCATION_KEY)
            {
                std::vector<t_token> new_tokens;
                for (std::vector<t_token>::iterator it2 = it + 1; it2 != tokens.end() && it2->type == VALUE; ++it2)
                    new_tokens.push_back(*it2);
                if (it->value == "allow_methods")
                {
                    if (z == 1)
                    {
                        print_error("allow_methods already set", &i);
                        break;
                    }
                    tempLocation.setAllowMethods(new_tokens, &i);
                    z = 1;
                }
                else if (it->value == "autoindex")
                    tempLocation.setAutoindex(new_tokens, &i);
                else if (it->value == "return")
                    tempLocation.setReturn(new_tokens, &i);
                else if (it->value == "root")
                    tempLocation.setRoot(new_tokens, &i);
                else if (it->value == "upload_dir")
                    tempLocation.setUploadDir(new_tokens, &i);
                else if (it->value == "index")
                    tempLocation.setIndex(new_tokens, &i);
                else if (it->value == "cgi")
                    tempLocation.setCgi(new_tokens, &i);
                else
                    print_error("unknown location key", &i);
                if (i == 1)
                    break;
            }
            else if (it->type == BLOCK)
            {
                if (it->value == "server")
                {
                    if (tokens.begin() != it || x == 1 || x == 2)
                    {
                        print_error("unexpected Server block", &i);
                        break;
                    }
                    x = 1;
                }
                else if (it->value == "location")
                {
                    if (tokens.begin() == it || (it + 1) == tokens.end() || (it + 1)->type != AFTER_BLOCK)
                    {
                        print_error("unexpected location block", &i);
                        break;
                    }
                    x = 2;
                    if (!tempLocation.getPath().empty())
                    {
                        z = 0;
                        if (!validatelocation(&i, tempLocation))
                        {
                            tempLocation = Config::Location();
                            break;
                        }
                        tempConfig.addLocation(tempLocation);
                        tempLocation = Config::Location();
                    }
                }
                else
                {
                    print_error("unknown block", &i);
                    break;
                }
            }
            else if (it->type == AFTER_BLOCK)
            {
                tempLocation.setPath(it->value, &i);
                if (i == 1)
                    break;
            }
            else if (it->type == VALUE)
                continue;
            else
                break;
        }
        if (i == 1)
        {
            tempConfig = Config();
            tempLocation = Config::Location();
            continue;
        }
        tempConfig.addLocation(tempLocation);
        if (!validateserver(tempConfig, &i) || !validatelocation(&i, tempLocation))
        {
            tempConfig = Config();
            tempLocation = Config::Location();
            continue;
        }
        this->configs.push_back(tempConfig);
        tempConfig = Config();
        tempLocation = Config::Location();
    }
    if (this->configs.empty())
    {
        std::cerr << "Error: all servers are invalid" << std::endl;
        exit(1);
    }
    // this->printConfig(this->configs);
}
