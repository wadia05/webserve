#ifndef WEBSRVE_HPP
#define WEBSRVE_HPP


#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>



#define BUFFER_SIZE 4096

class server {
    private :
        int port;
        int fd_server;
        int max_upload_size;
        std::string root; // server root directory
        std::string index_page; // html file  should be already uploded
        std::string error_page; // error 404 file should be already uploded
        sockaddr_in server_addr;
        

    public :
        server();
        ~server();
        std::string getRoot();
        std::string index();
        std::string error();
        void setupServer();

};



#endif