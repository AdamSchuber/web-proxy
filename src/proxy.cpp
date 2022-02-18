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
string get_ip_from_address(const char *address);
void split_address(string &address, string &port);

bool is_ok(const char *packet);
bool is_not_modified(const char *packet);

bool handle_if_image(char *request);
void modify_packet(char *packet, const char *old_word, const char *new_word);

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
            // Loops to find a appropriate request
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

        // closes client
        client.close_webserver();
    }
    return 0;
}

bool handle_if_image(char *request)
{
    const char *smiley{"smiley.jpg"};
    const char *trolly{"trolly.jpg"};
    const char *stockholm{"Stockholm-spring.jpg"};

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
                val = false;
            else
                val = true;
            break;
        }
    }
    tmp = 0;
    while (true)
    {
        // Searches for stockholm.jpg
        char *linkoping_ptr = strstr(request, stockholm);
        if (linkoping_ptr != NULL)
        {
            ++tmp;
            char new_request[2000];
            bzero((char *)new_request, sizeof(new_request));
            const char *get{"GET "};
            const char *url{"http://www.oskar000.se/buss/galleri/other/rystad_buss_juf344_linkoping_061124.jpg"};
            const char *protocoll{" HTTP/1.1\r\n"};
            const char *host{"Host: www.oskar000.se\r\n"};
            const char *user_agent{"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) Gecko/20100101 Firefox/97.0\r\n"};
            const char *accept_text{"Accept: image/avif,image/webp,*/*\r\n"};
            const char *accept_language{"Accept-Language: sv-SE,sv;q=0.8,en-US;q=0.5,en;q=0.3\r\n"};
            const char *accept_enc{"Accept-Encoding: gzip, deflate\r\n"};
            const char *connection{"Connection: keep-alive\r\n"};
            const char *upgrade{"Upgrade-Insecure-Requests: 1\r\n\r\n"};

            strcat(new_request, get);
            strcat(new_request, url);
            strcat(new_request, protocoll);
            strcat(new_request, host);
            strcat(new_request, user_agent);
            strcat(new_request, accept_text);
            strcat(new_request, accept_language);
            strcat(new_request, accept_enc);
            strcat(new_request, connection);
            strcat(new_request, upgrade);

            bzero((char *)request, sizeof(request));
            strcpy(request, new_request);
        }
        else
        {
            if (tmp == 0)
                val = false;
            else
                val = true;
            break;
        }
    }
    return val;
}

// Change old_word to new_word in the packet
void modify_packet(char *packet, const char *old_word, const char *new_word)
{
    char *link_ptr;
    char letter;
    while (true)
    {
        // packet_ptr points to first letter when finding old_word in packet
        char *packet_ptr = strstr(packet, old_word);
        if (packet_ptr != NULL)
        {
            // Check if image link
            char s = packet_ptr[-7];
            char r = packet_ptr[-6];
            char c = packet_ptr[-5];
            if (s == 's' && r == 'r' && c == 'c')
            {
                link_ptr = packet_ptr;
                letter = *link_ptr;
                *link_ptr = 'F';
                continue;
            }

            for (int i{}; i < strlen(old_word); i++)
                packet_ptr[i] = new_word[i];
        }
        else
        {
            *link_ptr = letter;
            break;
        }
    }
}

// Checks if packet got OK-code
bool is_ok(const char *packet)
{
    stringstream ss{packet};
    string buffer{};
    string OK{"200 OK"};
    getline(ss, buffer);
    auto start_it{search(buffer.begin(), buffer.end(), OK.begin(), OK.end())};
    if (start_it != buffer.end())
        return true;
    return false;
}

// Checks if packet got not modified code
bool is_not_modified(const char *packet)
{
    stringstream ss{packet};
    string buffer{};
    string not_modified{"304"};
    getline(ss, buffer);
    auto start_it{search(buffer.begin(), buffer.end(), not_modified.begin(), not_modified.end())};
    if (start_it != buffer.end())
        return true;
    return false;
}

// Finds the ip from the address in the GET recieved from browser
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

// Finds the address in the request recieved from browser
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