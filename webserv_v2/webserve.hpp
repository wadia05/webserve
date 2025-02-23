#ifndef WEBSRVE_HPP
#define WEBSRVE_HPP


#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>  // for strerror, memset
#include <cerrno>   // for errno
#include <unistd.h> // for close, read
#include <fcntl.h>  // for open
#include <vector>
#include <sstream> 


#define BUFFER_SIZE 4096

enum  Method {
    GET,
    POST,
    DELETE
};

class client {
    public :
        int fd_client;// in
        int fd_file; // out 
        sockaddr_in client_address;
        socklen_t client_len; 
        size_t fileSizeRead;
        Method method;
        std::string filePath;
        bool bodyFond;        
        char buffer[BUFFER_SIZE];
        bool finish;
        int vIndex;           

        // Add constructor
        client() : client_len(sizeof(client_address)), 
                  bodyFond(false), 
                  finish(false), 
                  vIndex(0) {}
};
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
        server();
        ~server();
        std::string getRoot(){return root;};
        std::string index(){return index_page;};
        std::string error(){return error_page;};
        std::string getServerIP() const;
        int getPort() const;


        void setupServer();
        void run_server();

        // void DELETE_handler(client &client, char *buffer);
        // void POST_handler(client &client, char *buffer);
        int GET_hander(client &client, char *buffer);
        void extractGET(client &client, char *buffer);   
        // int sender(client &Client, std::vector<client> &clients);
        int handler(client &client);
};




#endif