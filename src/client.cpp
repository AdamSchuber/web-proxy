#include "../include/client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>

using namespace std;

Client::Client(string const& ip, int const& port)
        : client{}, address{}, addr_size{}
    {
        if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::logic_error{"Socket creation error..."};

        std::cout << "Client socket has been created... " << std::endl;

        address.sin_family = AF_INET;
        address.sin_port = htons(port);


        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) <= 0)
            throw std::logic_error{"Invaild adress..."};

        if (connect(client, (struct sockaddr *)&address, sizeof(address)) == -1)
            throw std::logic_error{"Connection Failed..."};

        std::cout << "Awaiting confirmation from server... " << std::endl;
    }
    Client::~Client()
    {
        close(client);
    }

    string Client::transmit(const char* message)
    {
        send(client, message, strlen(message), 0);
        std::cout << "Message sent..." << std::endl;

        int valread = read(client, buffer, 16384);
        cout << "Message recived...";

        printf("%s\n", buffer);
        // cout << buffer << endl;

        return buffer;
    }