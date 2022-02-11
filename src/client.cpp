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
}

Client::~Client()
{
    close(client);
}

void Client::initialize_client(string const &ip)
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

char *Client::transmit(const char *message)
{
    send(client, message, strlen(message), 0);
    std::cout << "Message sent..." << std::endl;

    int check{1};
    char packet[200000];
    while (check > 0)
    {
        bzero((char *)buffer, sizeof(buffer));
        check = recv(client, buffer, sizeof(buffer), 0);
        printf("UNF_REQ: Received %d bytes\n", check);

        strcat(packet, buffer);

        if (check < 0)
        {
            fprintf(stderr, "%d: Unable to recieve data from server.\n", errno);
            close(client);
        }
    }
    cout << "Message received..." << endl;

    return packet;
}

// HTTP/1.1 200 OK
// Date: Fri, 11 Feb 2022 08:15:23 GMT
// Server: Apache/2.4.6 (CentOS) mod_auth_gssapi/1.5.1 mod_nss/1.0.14 NSS/3.28.4 mod_wsgi/3.4 Python/2.7.5
// Last-Modified: Fri, 15 Jan 2021 11:35:43 GMT
// ETag: "2c5-5b8eec5c5e21a"
// Accept-Ranges: bytes
// Content-Length: 709
// Keep-Alive: timeout=5, max=100
// Connection: Keep-Alive
// Content-Type: text/html; charset=UTF-8

// <html>

// <title>
// HTML page with a linked photo of Smiley for Fake News assignment
// </title>

// <body>

// <h1>An HTML page with link to image of Smiley from Stockholm</h1>

// <p>
//   This is a more complicated Web page, with a link to a photo.
//   It tells a simple story about our yellow friend Smiley,
//   who is from Stockholm. Smiley is round, I think.
// </p>
// <p>
// Here is an <a href="./smiley.jpg">image</a> of Smiley from Stockholm.
// </p>
// <p>
// Without your proxy, you should be able to view this page just fine.
// </p>
// <p>
// With your proxy, this page should look a bit different,
// with all mentions of Smiley from Stockholm
// being changed into something else.
// The page should still be formatted properly.
// </p>

// </body>

// </html>