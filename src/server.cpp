#include "../include/server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

using namespace std;

Server::Server()
    : address{}, listening{}, browser{}, addr_size{sizeof(address)}
{
    if ((listening = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::logic_error{"Error stablishing socket..."};

    setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    std::cout << "Server socket has been created... " << std::endl;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htons(INADDR_ANY);
    address.sin_port = htons(PORT);

    if ((bind(listening, (struct sockaddr *)&address, addr_size)) < 0)
        throw std::logic_error{"Error binding connection..."};

    if (listen(listening, SOMAXCONN) == 1)
        throw std::logic_error{"listen failed"};
}
Server::~Server()
{
    close(listening);
}

std::string Server::get_request()
{
    std::string request{};
    browser = accept(listening, (struct sockaddr *)&address, &addr_size);
    if (browser < 0)
        throw std::logic_error{"Error on accepting... "};

    int valread = read(browser, buffer, 1024);
    request = buffer;
    return request;
}

void Server::transmit(const char* packet)
{
    if (send(browser, packet, strlen(packet), 0) == 1)
        throw std::logic_error{"Send failed"};

    std::cout << "Packet sent..." << std::endl;    
    close(browser);
}