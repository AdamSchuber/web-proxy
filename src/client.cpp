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

Client::Client()
    : client{}, address{}, addr_size{}, ip{}
{
    if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw logic_error{"Socket creation error..."};

    // setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    cout << "Client socket has been created... " << endl;

    // Set ivp and port number
    address.sin_family = AF_INET;
    address.sin_port = htons(80);
}

void Client::connect_to_webserver(string const &ip)
{
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) <= 0)
        throw logic_error{"Invaild adress..."};

    // Connect client
    if (connect(client, (struct sockaddr *)&address, sizeof(address)) == -1)
        throw logic_error{"Connection Failed..."};

    cout << "Awaiting confirmation from server... " << endl;
}

void Client::close_webserver()
{
    close(client);
}

ssize_t Client::transmit(const char *message, char *packet)
{
    // Send http request to server
    send(client, message, strlen(message), 0);
    cout << "Message sent from client..." << endl;

    // cout << message << endl;

    // Recive binary data into buffer
    bzero((char *)buffer, sizeof(buffer));
    int size = recv(client, buffer, sizeof(buffer), MSG_WAITALL);
    printf("UNF_REQ: Received %d bytes\n", size);

    // Copy buffer data into packet
    memcpy(packet, buffer, size);

    if (size < 0)
    {
        fprintf(stderr, "%d: Unable to recieve data from server.\n", errno);
        close(client);
    }

    cout << "Message received to client..." << endl;
    return size;
}