#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector>

class Client
{
public:
    Client();
    ~Client();
    ssize_t transmit(const char* message, char* packet);
    void initialize_client(std::string const& ip);
private:
    char buffer[500000];
    int client;
    std::string ip;
    struct sockaddr_in address;
    socklen_t addr_size;
};

#endif /*CLIENT_H*/