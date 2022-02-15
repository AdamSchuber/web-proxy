#include "../include/server.h"
#include "../include/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <stdexcept>
using namespace std;

string get_address(string const &request);
void split_address(string &address, string &port);
string get_ip_from_address(const char *address);
bool is_ok(const char *packet);
bool is_not_modified(const char *packet);

int main()
{
    Server server{};
    Client client{};
    while (true)
    {
        char packet[500000];
        string request{}, address{}, ip{};
        do
        {
            try
            {
                // Decode IP-address
                request = server.get_request();
                address = get_address(request);
                ip = get_ip_from_address(address.c_str());

                // Initiate client with decoded information until valid address
                client.connect_to_webserver(ip);
                break;
            }
            catch (const exception &e)
            {
                cout << address << endl;
                cerr << "Invalid address..." << endl;
            }
        } while (true);

        // Get packet from web-server
        const char *message{request.c_str()};
        bzero((char *)packet, sizeof(packet));
        ssize_t size{client.transmit(message, packet)};

        //cout << packet << endl;

        // Send packet to browser from proxy-server
        if (is_ok(packet) || is_not_modified(packet))
            server.transmit(packet, size);

        client.close_webserver();
    }
    return 0;
}

bool is_ok(const char *packet)
{
    stringstream ss{packet};
    string buffer{};
    string OK{"200 OK"};
    getline(ss, buffer);
    auto start_it{search(buffer.begin(), buffer.end(), OK.begin(), OK.end())};
    if (start_it != buffer.end())
    {
        return true;
    }
    return false;
}

bool is_not_modified(const char *packet)
{
    stringstream ss{packet};
    string buffer{};
    string not_modified{"304"};
    getline(ss, buffer);
    auto start_it{search(buffer.begin(), buffer.end(), not_modified.begin(), not_modified.end())};
    if (start_it != buffer.end())
    {
        return true;
    }
    return false;
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
