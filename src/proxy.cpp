#include "../include/server.h"
#include "../include/client.h"
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

string get_address(string & request);

int main()
{
    int a;
    cout << "hej" << endl;
    cin >> a;
    /* Server server{};
    string request{server.get_request()};

    // decode IP-address
    cout << request << endl;
    cout << endl;
    string address{get_address(request)};

    // initiate client with decoded information
    Client client{};
    const char* message{request.c_str()};
    client.transmit(message); */

    return 0;
}

string get_address(string & request)
{
    stringstream ss{request};
    string adress_and_port{};
    string buffer{};
    const string host{"Host:"};
    
        // Get host line
    while(getline(ss, buffer))
    {
        auto start_it{search(buffer.begin(), buffer.end(), host.begin(), host.end())};
        if ( start_it != buffer.end())
        {
            string str{start_it+6, buffer.end()};
            adress_and_port = str;
        }
    }
    cout << adress_and_port << endl;

    // Spilt Host: adress, and PORT


    // Translate adress to IP
    

    return "address";
}