#include "../include/client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>

using namespace std;

Client::Client(string const& ip)
        : client{}, address{}, addr_size{}
    {
        if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::logic_error{"Socket creation error..."};

        std::cout << "Client socket has been created... " << std::endl;

        address.sin_family = AF_INET;
        address.sin_port = htons(80);       //PORT = 80

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

        // int reading = read(client, buffer, 2048);
        // cout << buffer << endl;
        
        while (true)
        {
            ssize_t check{read(client, buffer, 2048)};
            cout << buffer << endl;
            if (check <= 0)
                break;
        }

        //cout << "Message received..." << endl;

        return buffer;
    }




  /*   int byte_counter{};
        vector<int> bytes_read{};
        while (byte_counter < 170970)
        {
            ssize_t check{read(client, buffer, 4096)};
            cout << buffer << endl;
            bytes_read.push_back(sizeof(buffer));
            byte_counter += sizeof(buffer);
        }

        for(int i{}; i < bytes_read.size(); ++i)
        {
            cout << "byte " << i << ": " << bytes_read.at(i) << endl;
        } */