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
                // image = handle_if_image(request);
                address = get_address(request);
                ip = get_ip_from_address(address.c_str());

                cout << request << endl;

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
        // if (!image)
        // {
        //     modify_packet(packet, "Stockholm", "Linkoping");
        //     modify_packet(packet, "Smiley", "Trolly");
        // }

        cout << packet << endl;

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
    // const char *stockholm{"Stockholm-spring.jpg"};
    const char *stockholm{"Linkoping-spring.jpg"};
    const char *linkoping{"http://naturkartan-images.imgix.net/image/upload/jv1xkiprxn1fuvlg2amg/1408440053.jpg"};

    bool val{false};
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
            {
                val = false;
            }
            else
            {
                val = true;
            }
            break;
        }
    }
    tmp = 0;
    while (true)
    {
        char *linkoping_ptr = strstr(request, stockholm);
        if (linkoping_ptr != NULL)
        {
            ++tmp;
            string new_request{"GET "};
            new_request += linkoping;
            new_request += " HTTP/1.1 \n";
            new_request += "Host: naturkartan-images.imgix.net \n";
            new_request += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) Gecko/20100101 Firefox/97.0 \n";
            new_request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8 \n";
            new_request += "Accept-Language: sv-SE,sv;q=0.8,en-US;q=0.5,en;q=0.3 \n";
            new_request += "Accept-Encoding: gzip, deflate \n";
            new_request += "Connection: keep-alive \n";
            new_request += "Upgrade-Insecure-Requests: 1 \n \n";
            
            strcpy(request, new_request.c_str());
        }
        else
        {
            if (tmp == 0)
            {
                val = false;
            }
            else
            {
                val = true;
            }
            break;
        }
    }
    return val;
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