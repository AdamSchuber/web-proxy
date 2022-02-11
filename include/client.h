#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>

class Client
{
public:
    Client();
    ~Client();
    std::string transmit(const char* message);
    void initialize_client(std::string const& ip);
private:
    char buffer[10000];
    int client;
    struct sockaddr_in address;
    socklen_t addr_size;
};

#endif /*CLIENT_H*/