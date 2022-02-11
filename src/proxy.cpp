#include "../include/server.h"
#include "../include/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

string get_address(string const &request);
void split_address(string &address, string &port);
string get_ip_from_address(const char *address);

int main()
{
    Server server{};
    Client client{};
    char packet[200000];
    do
    {
        // decode IP-address
        string request{server.get_request()};
        string address{get_address(request)};
        string ip{get_ip_from_address(address.c_str())};
        
        // initiate client with decoded information
        client.initialize_client(ip);

        // get packet from web-server
        const char *message{request.c_str()};
        bzero((char *)packet, sizeof(packet));
        //strcat(packet, client.transmit(message));
        //client.transmit(message)
        printf(client.transmit(message));

        // send packet to browser from proxy-server
        server.transmit(packet);
    } while(true);

    return 0;
}

string get_ip_from_address(const char *address)
{
    struct addrinfo hints, *res;
    char host[16384];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    int ret = getaddrinfo(address, NULL, &hints, &res);

    for (auto tmp = res; tmp != NULL; tmp = tmp->ai_next)
    {
        getnameinfo(tmp->ai_addr, tmp->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    }
    return host;
}

string get_address(string const &request)
{
    stringstream ss{request};
    string address{};
    string buffer{};
    const string host{"Host:"};

    // Get host line
    while (getline(ss, buffer))
    {
        auto start_it{search(buffer.begin(), buffer.end(), host.begin(), host.end())};
        if (start_it != buffer.end())
        {
            string str{start_it + 6, buffer.end() - 1};
            address = str;
        }
    }
    return address;
}
