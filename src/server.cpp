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
    // Creates listening socket
    if ((listening = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw logic_error{"Error stablishing socket..."};

    setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    cout << "Server socket has been created... " << endl;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htons(INADDR_ANY);
    address.sin_port = htons(PORT);

    if ((bind(listening, (struct sockaddr *)&address, addr_size)) < 0)
        throw logic_error{"Error binding connection..."};

    if (listen(listening, SOMAXCONN) == 1)
        throw logic_error{"listen failed"};
}

Server::~Server()
{
    close(listening);
    close(browser);
}

void Server::get_request(char *request)
{
    browser = accept(listening, (struct sockaddr *)&address, &addr_size);
    if (browser < 0)
        throw logic_error{"Error on accepting... "};
        
    // Recives request from browser
    if (recv(browser, buffer, sizeof(buffer), 0) < 0)
        throw logic_error{"Read failed... "};

    // Copies buffer to request
    strcpy(request, buffer);
}

void Server::transmit(const char *packet, ssize_t const &size)
{
    // Send full packet
    if (send(browser, packet, size, MSG_WAITALL) < 0)
        throw logic_error{"Send failed"};

    cout << "Packet sent..." << endl;
}