#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

int main() {
    char buffer[100];
    std::vector<int> fds;
    
    // Open input files
    fds.push_back(open("test.cpp", O_RDONLY));
    if (fds.back() == -1) {
        perror("open");
        return 1;
    }
    
    fds.push_back(open("websrve.hpp", O_RDONLY));
    if (fds.back() == -1) {
        perror("open");
        return 1;
    }
    
    // Open output files
    std::vector<int> fdc;
    fdc.push_back(open("test0", O_CREAT | O_WRONLY | O_TRUNC, 0644));
    if (fdc.back() == -1) {
        perror("open");
        return 1;
    }
    
    fdc.push_back(open("test1", O_CREAT | O_WRONLY | O_TRUNC, 0644));
    if (fdc.back() == -1) {
        perror("open");
        return 1;
    }
    
    // Read and write characters
    ssize_t bytes_read;
    while (!fds.empty()) {
        for (size_t i = 0; i < fds.size(); ) {
            bytes_read = read(fds[i], buffer, sizeof(buffer));
            if (bytes_read > 0) {
                write(fdc[i], buffer, bytes_read);
                i++;
            } else {
                close(fds[i]);
                fds.erase(fds.begin() + i);
            }
        }
    }
    
    // Close all files
    for (size_t i = 0; i < fds.size(); i++) {
        close(fds[i]);
    }
    
    for (size_t i = 0; i < fdc.size(); i++) {
        close(fdc[i]);
    }
    
    return 0;
}