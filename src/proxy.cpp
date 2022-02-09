#include "../include/server.h"
#include "../include/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

string get_address(string const& request);
void split_address(string & address, string & port);
string get_ip_from_address(const char*  address);

int main()
{
    Server server{};
    string request{server.get_request()};

        // decode IP-address
    string address{get_address(request)};
    string port{};
    split_address(address, port);
    int port_int{stoi(port)};
    string ip{get_ip_from_address(address.c_str())};

        // initiate client with decoded information
    Client client{ip, port_int};

    const char* message{request.c_str()};
    string packet = client.transmit(message);
    
    return 0;
}

string get_ip_from_address(const char* address)
{
    struct addrinfo hints, *res;
    char host[16384];
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    int ret = getaddrinfo(address, NULL, &hints, &res);
    
    for (auto tmp = res; tmp != NULL; tmp = tmp->ai_next) {
        getnameinfo(tmp->ai_addr, tmp->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    }
    return host;
}

void split_address(string & address, string & port)
{
    auto seperate_it{find(address.begin(), address.end(), ':')};
    string temp{address.begin(), seperate_it};
    string temp2{seperate_it+1, address.end()};
    address = temp;
    port = temp2;
}

// string get_address(string const& request)
// {
//     stringstream ss{request};
//     string address_and_port{};
//     string buffer{};
//     const string host{"Host:"};
    
//         // Get host line
//     while(getline(ss, buffer))
//     {
//         auto start_it{search(buffer.begin(), buffer.end(), host.begin(), host.end())};
//         if ( start_it != buffer.end())
//         {
//             string str{start_it+6, buffer.end()};
//             address_and_port = str;
//         }
//     }
//     return address_and_port;
// }

string get_address(string const& request)
{    
    auto it = find(request.begin(), request.end(), 'H');
    auto it2 = find(it, request.end(), '\'');
    string address_and_port{it+6, it2};

    cout << address_and_port << endl;

    return address_and_port;
}