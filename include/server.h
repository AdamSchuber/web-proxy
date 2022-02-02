#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

#define PORT 8080

class Server
{
public:
    Server()
        : address{}, listening{}, addr_size{sizeof(address)}
    {
        if ((listening = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::logic_error{"Error stablishing socket..."};

        setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

        std::cout << "Server socket has been created... " << std::endl;

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htons(INADDR_ANY);
        address.sin_port = htons(PORT);

        if ((bind(listening, (struct sockaddr *)&address, addr_size)) < 0)
            throw std::logic_error{"Error binding connection..."};

        if (listen(listening, SOMAXCONN) == 1)
            throw std::logic_error{"listen failed"};
    }
    ~Server()
    {
        close(listening);
    }

    std::string get_request()
    {
        std::string request{};
        int browser = accept(listening, (struct sockaddr *)&address, &addr_size);
        if (browser < 0)
            throw std::logic_error{"Error on accepting... "};

        int valread = read(browser, buffer, 1024);
        request = buffer;
        close(browser);
        return request;
    }

private:
    char buffer[1024];
    struct sockaddr_in address;
    int listening;
    socklen_t addr_size{};
    int one{1};
};

#endif /*SERVER_H*/