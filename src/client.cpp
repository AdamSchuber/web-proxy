#include "../include/client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector> //temporär
#include <sstream>

using namespace std;

Client::Client(string const &ip)
    : client{}, address{}, addr_size{}
{
    if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::logic_error{"Socket creation error..."};

    std::cout << "Client socket has been created... " << std::endl;

    address.sin_family = AF_INET;
    address.sin_port = htons(80); // PORT = 80

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

string Client::transmit(const char *message)
{
    send(client, message, strlen(message), 0);
    std::cout << "Message sent..." << std::endl;

    // int reading = read(client, buffer, 2048);
    // cout << buffer << endl;
    // bzero((char *)buffer, 200000);
    // ssize_t check{recv(client, buffer, 200000, 0)};
    int check{1};
    string packet{};
    while (check > 0)
    {
        bzero((char *)buffer, 10000);
        usleep(100);
        check = recv(client, buffer, 10000, 0);
        printf("UNF_REQ: Received %d bytes\n", check);
        cout << buffer;
        packet += buffer;

        if (check < 0) {
            fprintf(stderr, "%d: Unable to recieve data from server.\n", errno);
            close(client);
        }
    }
    cout << "Message received..." << endl;

    return packet;
}

// // If our proxy doesn't get a HTTP/1.1 or 1.0 GET request,
// // we process the request without any filtering.
// void unfiltered_request(int request_fd, char *buffer, int client_fd)
// {
//     int n = 1;
//     printf("Doing unfiltered request\n");
//     while (n > 0)
//     {
//         bzero((char *)buffer, 10000);
//         n = recv(request_fd, buffer, 10000, 0);
//         printf("UNF_REQ: Received %d bytes\n", n);

//         if (n < 0)
//         {
//             fprintf(stderr, "%d: Unable to recieve data from server.\n", errno);
//             close(request_fd);
//             return;
//         }

//         sendall(client_fd, buffer, &n);
//         printf("UNF_REQ: Sent %d bytes\n", n);
//     }

//     close(request_fd);
//     return;
// }

 /*  int byte_counter{};
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