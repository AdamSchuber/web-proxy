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

string get_address(char *request);
void split_address(string &address, string &port);
string get_ip_from_address(const char *address);
bool is_ok(const char *packet);
bool is_not_modified(const char *packet);
void modify_packet(char *packet, const char *old_word, const char *new_word);
bool is_text(char *packet);
bool handle_if_image(char *request);

int main()
{
    while (true)
    {
        Server server{};
        Client client{};
        char packet[500000];
        char request[20000];
        string address{}, ip{};
        bool image = false;
        do
        {
            try
            {
                // Decode IP-address
                server.get_request(request);
                image = handle_if_image(request);
                address = get_address(request);
                ip = get_ip_from_address(address.c_str());

                // Initiate client with decoded information until valid address
                client.connect_to_webserver(ip);
                break;
            }
            catch (const exception &e)
            {
                cerr << e.what() << endl;
            }
        } while (true);

        // Get packet from web-server
        bzero((char *)packet, sizeof(packet));
        ssize_t size{client.transmit(request, packet)};

        // Modifies return packet
        if (!image)
        {
            modify_packet(packet, "Stockholm", "Linkoping");
            modify_packet(packet, "Smiley", "Trolly");
        }

        // Send packet to browser from proxy-server
        if (is_ok(packet) || is_not_modified(packet))
            server.transmit(packet, size);

        client.close_webserver();
    }
    return 0;
}

bool handle_if_image(char *request)
{
    const char *smiley{"smiley.jpg"};
    const char *trolly{"trolly.jpg"};
    int tmp{};
    while (true)
    {
        char *smiley_ptr = strstr(request, smiley);
        if (smiley_ptr != NULL)
        {
            ++tmp;
            for (int i{}; i < strlen(smiley); i++)
                smiley_ptr[i] = trolly[i];
        }
        else
        {
            if (tmp == 0)
                return false;
            return true;
        }
    }
    return false;
}

void modify_packet(char *packet, const char *old_word, const char *new_word)
{
    while (true)
    {
        char *packet_ptr = strstr(packet, old_word);
        if (packet_ptr != NULL)
        {
            for (int i{}; i < strlen(old_word); i++)
                packet_ptr[i] = new_word[i];
        }
        else
        {
            break;
        }
    }
}

// bool is_text(const char *packet)
// {
//     stringstream ss{packet};
//     string buffer{};
//     const string image{"image/jpeg"};

//     // contains image?
//     while (getline(ss, buffer))
//     {
//         auto start_it{search(buffer.begin(), buffer.end(), image.begin(), image.end())};
//         if (start_it != buffer.end())
//             return false; // It does!
//     }
//     return true;
// }

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

string get_address(char *request)
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