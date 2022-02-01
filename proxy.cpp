#include "server.h"
#include <iostream>

using namespace std;

int main()
{
    Server server{};
    string str{server.get_request()};
    cout << str << endl;
    return 0;
}













































/* char buffer[1024] = {0};
char *hello = "Hello from server";
struct sockaddr_in address;
socklen_t addr_size = sizeof(address);
int listening = socket(AF_INET, SOCK_STREAM, 0);
if (listening < 0)
{
    cout << "\nError establishing socket..." << endl;
    return -1;
}

cout << "Socket server has been created... " << endl;

address.sin_family = AF_INET;
address.sin_addr.s_addr = htons(INADDR_ANY);
address.sin_port = htons(PORT);

if ((bind(listening, (struct sockaddr *)&address, addr_size)) < 0)
{
    cout << "Error binding connection... " << endl;
    return -1;
}

if (listen(listening, SOMAXCONN) == 1)
{
    cout << "listen failed" << endl;
    return -1;
}
*/
// int browser = accept(listening, (struct sockaddr *)&address, &addr_size);
// if (browser < 0)
// {
//     cout << "Error on accepting... " << endl;
//     return -1;
// }
// while (browser > 0)
// {
//     int valread = read(browser, buffer, 1024);
//     cout << buffer << endl;

//     // send(new_socket, hello, strlen(hello), 0);
//     // cout << "Message sent" << endl;
// }

// https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html