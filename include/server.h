#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#define PORT 1234

class Server
{
public:
    Server();
    ~Server();

    std::string get_request();
    void transmit(const char* packet, ssize_t const& size);
    bool contains_image(const char* packet);
    int sendall(int socket, const char* packet, int *len);
private:
    char buffer[8192];
    struct sockaddr_in address;
    int listening, browser;
    socklen_t addr_size{};
    int one{1};
};

#endif /*SERVER_H*/