#include "../include/server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>

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

    int valread = read(browser, buffer, 16384);
    request = buffer;
    return request;
}

void Server::transmit(const char *packet)
{
    int temp = strlen(packet);
    int *size = &temp;
    int n{};

    n = sendall(browser, packet, size);

    // if (contains_image(packet))
    // {
    //     n = sendall(browser, packet, size);
    // }
    // else if (send(browser, packet, strlen(packet), 0) == 1)
    // {
    //     throw std::logic_error{"Send failed"};
    // }

    printf(packet);

    std::cout << "Packet sent..." << std::endl;
}

bool Server::contains_image(const char *packet)
{
    stringstream ss{packet};
    string buffer{};
    const string image{"image/jpeg"};

    // contains image?
    while (getline(ss, buffer))
    {
        auto start_it{search(buffer.begin(), buffer.end(), image.begin(), image.end())};
        if (start_it != buffer.end())
            return true; // It does!
    }
    return false;
}

int Server::sendall(int socket, const char *packet, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while (total < *len)
    {
        n = send(socket, packet + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}