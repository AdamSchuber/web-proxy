#include "../include/client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>

#define PORT 8080

Client::Client()
        : client{}, address{}, addr_size{}
    {
        if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::logic_error{"Socket creation error..."};

        std::cout << "Client socket has been created... " << std::endl;

        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);


        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0)
            throw std::logic_error{"Invaild adress..."};

        if (connect(client, (struct sockaddr *)&address, sizeof(address)) == -1)
            throw std::logic_error{"Connection Failed..."};

        std::cout << "Awaiting confirmation from server... " << std::endl;
    }
    Client::~Client()
    {
        close(client);
    }

    void Client::transmit(const char* message)
    {
        send(client, message, strlen(message), 0);
        std::cout << "Message sent..." << std::endl;
        //int valread = read(client, buffer, 1024);
        //printf("%s\n", buffer);
    }