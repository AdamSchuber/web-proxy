#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

#define PORT 8080

using namespace std;

int main()
{
    int client{0}, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    cout << "Socket has been created... " << endl;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        cout << "\nInvalid address/ Address not supported " << endl;
        return -1;
    }

    if (connect(client, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    cout << "Awaiting confirmation from server... " << endl;

    send(client , hello , strlen(hello) , 0);
    cout << "Hello message sent... " << endl;;
    valread = read( client , buffer, 1024);
    printf("%s\n",buffer );

    return 0;
}