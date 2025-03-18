#pragma once

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD "\033[1m"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <set>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "./configFile/Tokenizer.hpp"
#include "./configFile/Config.hpp"
#include "./req/HTTPRequest.hpp"

class Config;
class HTTPRequest;


#define BUFFER_SIZE 1024
#define MAX_EVENTS 20

enum  Method {
    GET,
    POST,
    DELETE,
    NOTDETECTED
};
enum  PostType {
    MULTI,
    CHANKED,
    SIMPLE,
    NON
};
int printer(std::vector<std::string> parts);
std::vector<std::string> split(const std::string& str, const std::string& delimiter) ;
class client {
    public :
        int fd_client;// in
        int fd_file; // out 
        std::ifstream* file;
        std::string G_P_Responce; // this variable store the get responce or post requist
        sockaddr_in client_address;
        socklen_t client_len; 
        size_t fileSizeRead;
        Method method;
        std::string filePath;
        size_t  fullfileSize;
        bool hedersend;
        bool bodyFond;        
        char buffer[BUFFER_SIZE];
        bool finish;
        int vIndex;           

        // Add constructor
        client() : fd_client(0), fd_file(0), file(NULL), client_len(sizeof(client_address)),
            fileSizeRead(0), method(NOTDETECTED), fullfileSize(0), hedersend(false),
            bodyFond(false), finish(false), vIndex(0) {}
        ~client() {
            // if (file != NULL) {
            //     if (file->is_open())
            //         file->close();
            //     delete file;
            // }
        }
                
        

    };
// std::string resposePrepare(client &Client, char *buffer);
std::string prepareErrorResponse(int statusCode);
std::string prepareResponseHeaders(client &Client);
class server {
    private :
        int port;
        int fd_server;
        int max_upload_size;
        std::string serverName;
        std::string serverIp;
        std::string root; // server root directory
        std::string index_page; // html file  should be already uploded
        std::string error_page; // error 404 file should be already uploded
        sockaddr_in server_addr;


    public :
        server(const Config &config);
        ~server();
        std::string getRoot(){return root;};
        std::string index(){return index_page;};
        std::string error(){return error_page;};
        std::string getServerIP() const;
        int getPort() const;


        void setupServer();
        void run_server();

        // void DELETE_handler(client &client, char *buffer);
        int POST_handler(client &client, HTTPRequest &request);
        int GET_hander(client &client, HTTPRequest &request);
            // void extractGET(client &client);   
        // int sender(client &Client, std::vector<client> &clients);
        int handler(client &client);
};
