/*
** proxy.c -- A nice little proxy server called Net Ninny
** Authors: Eric Henziger, erihe763@student.liu.se
** Olof Rappestad, olora937@stundet.liu.se
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 8080

// uncomment to disable prints and make stuff faster!
//#define printf(...)

// Helper method for reaping zombie processes.
void sigchld_handler(int s)
{
    //
}

// get sockaddr, IPv4 or IPv6.
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Try your best to send all the data!
int sendall(int socket, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while (total < *len)
    {
        n = send(socket, buf + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// If our proxy doesn't get a HTTP/1.1 or 1.0 GET request,
// we process the request without any filtering.
void unfiltered_request(int request_fd, char *buffer, int client_fd)
{
    int n = 1;
    printf("Doing unfiltered request\n");
    while (n > 0)
    {
        bzero((char *)buffer, 10000);
        n = recv(request_fd, buffer, 10000, 0);
        printf("UNF_REQ: Received %d bytes\n", n);

        if (n < 0)
        {
            fprintf(stderr, "%d: Unable to recieve data from server.\n", errno);
            close(request_fd);
            return;
        }

        sendall(client_fd, buffer, &n);
        printf("UNF_REQ: Sent %d bytes\n", n);
    }

    close(request_fd);
    return;
}

// Generic filter function that search for the strings stored
// in bad_strings within the buffer. Case insensitive.
// Returns 1 if bad string is found, otherwise 0.
int filter(char *buffer)
{
    char *cptr = NULL;
    char *bad_strings[4];
    bad_strings[0] = "spongebob";
    bad_strings[1] = "Britney Spears";
    bad_strings[2] = "Paris Hilton";
    bad_strings[3] = "NorrkÃ¶ping";
    int i;
    for (i = 0; i < 3; i++)
    {
        if (buffer != NULL)
        {
            cptr = strcasestr(buffer, bad_strings[i]);
            if (cptr != NULL)
            {
                return 1;
            }
        }
    }
    return 0;
}

// Searches the host and path values from a http header and
// looks for bad strings.
// Returns 1 if bad string is found, otherwise 0.
int url_filter(char *buffer, char *path)
{
    char *cptr = NULL;
    char *host = NULL;

    cptr = strstr(buffer, "\nHost: ");
    if (cptr != NULL)
    {
        cptr += 7; // Increase past the "Host: " part
        host = strtok(cptr, "\r\n");
        printf("Value from Host header: %s\n", host);
    }

    if (filter(path) || filter(host))
        return 1;
    return 0;
}

// Extracts the value for the header that follows the @header_pattern pattern.
// Returns the value, or NULL if not found.
char *get_header_info(char *buffer, char *header_pattern, int offset)
{
    char *ptr = NULL;
    char *value = NULL;
    ptr = strstr(buffer, header_pattern);
    if (ptr != NULL)
    {
        // We found the header pattern, now extract the value.
        ptr += offset; // Increase ptr to the value part.
        value = strtok(ptr, "\r\n");
    }

    return value;
}

// Seachers the buffer for Content-type header with a value of text/
// Will return 1 if we find it, otherwise 0.
int is_content_text(char *buffer)
{
    char *cptr = NULL, *type = NULL;
    cptr = strcasestr(buffer, "\nContent-type: text/");
    if (cptr != NULL)
    {
        return 1;
    }
    return 0;
}

// Seachers the buffer for Content-Encoding header with a value of gzip
// Will return 1 if we find it, otherwise 0.
int is_encoding_gzip(char *buffer)
{
    char *cptr = NULL;
    cptr = strcasestr(buffer, "\nContent-Encoding: gzip");
    if (cptr != NULL)
    {
        return 1;
    }
    return 0;
}

// Wrapper for the filter method to indicate that we're doing filtering
// on the content of a web page.
int content_filter(char *buffer)
{
    if (filter(buffer))
        return 1;
    return 0;
}

// Large method for handling all communication from the client
// to the server and back. Takes a client socket as argument.
// Returns when communication is done, or on unexpected error.
void handle_client_request(int client_fd)
{
    char buffer[10000], method[10000], uri[10000], ver[10000],
        user_agent[3000],
        accept[3000], accept_lang[3000], accept_enc[3000];
    char *temp = NULL;
    int request_fd; // Socket for the the communication to the server.
    int do_unfiltered_request = 0;
    struct hostent *host;
    struct sockaddr_in host_addr;

    // Fill the buffer with zero-valued bytes.
    bzero((char *)buffer, 10000);

    // Receive the request from the client and save it to our buffer.
    int recved = recv(client_fd, buffer, 10000, 0);

    printf("### RECEIEVED %d BYTES FROM CLIENT REQUEST\n", recved);

    // Check the first line of the request and split it into smaller parts.
    // [METHOD] [HOST/PATH] [PROTOCOL ver.]
    sscanf(buffer, "%s %s %s", method, uri, ver);

    // Verify that we have a GET request and we're using HTTP 1.0 or 1.1
    if ((strncmp(method, "GET", 3) == 0) &&
        ((strncmp(ver, "HTTP/1.1", 8) == 0) ||
         (strncmp(ver, "HTTP/1.0", 8) == 0)))
    {
    }
    else
    {
        printf("UNFILTER: ###%s!!!%s===%s@@@\n", method, uri, ver);
        do_unfiltered_request = 1;
    }

    // Write uri to method because we use the uri twice below,
    // but we destroy it with strtok.
    strcpy(method, uri);

    // Expect URI to possibly be like http://www.example.com/path/index.html
    // Separate the URI part of the request.
    temp = strtok(uri, "//");
    // After the following strtok, temp is only the host part
    // (www.example.com)
    temp = strtok(NULL, "/");

    sprintf(uri, "%s", temp);
    printf("host = %s", uri);
    // Get IP address from DNS hostname.
    host = gethostbyname(uri);

    strcat(method, "^]");
    temp = strtok(method, "//");
    temp = strtok(NULL, "/");
    if (temp != NULL)
        temp = strtok(NULL, "^]");
    printf("\npath = %s\nPort = 80\n", temp);

    // Save header values found in request, this destroys the buffer.
    strcpy(accept_enc, get_header_info(buffer, "\nAccept-Encoding:", 17));
    strcpy(accept_lang, get_header_info(buffer, "\nAccept-Language:", 17));
    strcpy(accept, get_header_info(buffer, "\nAccept:", 8));
    strcpy(user_agent, get_header_info(buffer, "\nUser-Agent:", 12));

    // Filter the request based on URL. Send 302 redirect if the filter
    // finds inappropriate content.
    if (url_filter(buffer, temp))
    {
        int rc = send(client_fd, "HTTP/1.1 302 Found\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html\r\n\r\n", 96, 0);
        if (rc < 0)
        {
            fprintf(stderr, "%d: Unable to send bad url page redirect to client.\n", errno);
        }
        return;
    }

    // Create a socket for communicating with the web server.
    bzero((char *)&host_addr, sizeof(host_addr));
    host_addr.sin_port = htons(80);
    host_addr.sin_family = AF_INET;
    bcopy((char *)host->h_addr,
          (char *)&host_addr.sin_addr.s_addr,
          host->h_length);

    request_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(request_fd,
                (struct sockaddr *)&host_addr,
                sizeof(struct sockaddr)) == -1)
    {
        close(request_fd);
        perror("Error: Unable to create server socket");
        return;
    }

    sprintf(buffer, "\nConnected to %s (%s)\n", uri,
            inet_ntoa(host_addr.sin_addr));

    // Send a GET request with our selected choice of HTTP headers.
    bzero((char *)buffer, sizeof(buffer));
    if (temp != NULL)
        sprintf(buffer, "GET /%s %s\r\nHost: %s\r\nUser-Agent:%s\r\nAccept:%s\r\nAccept-Language:%s\r\nAccept-Encoding:%s\r\nConnection: close\r\n\r\n", temp, ver, uri, user_agent, accept, accept_lang, accept_enc);
    else
        sprintf(buffer, "GET / %s\r\nHost: %s\r\nUser-Agent:%s\r\nAccept:%s\r\nAccept-Language:%s\r\nAccept-Encoding:%s\r\nConnection: close\r\n\r\n", ver, uri, user_agent, accept, accept_lang, accept_enc);

    printf("\n%s\n", buffer);

    int n = send(request_fd, buffer, strlen(buffer), 0);
    if (n == -1)
    {
        perror("Unable to send request to remote host\n");
    }

    if (do_unfiltered_request == 1)
    {
        unfiltered_request(request_fd, buffer, client_fd);
        return;
    }

    // Start handling the content received from the server.
    int is_text = 0;
    char text_buffer[8000000];
    int message_length = 0;
    bzero((char *)text_buffer, 8000000);

    while (n > 0)
    {
        bzero((char *)buffer, 10000);
        n = recv(request_fd, buffer, 10000, 0);

        if (is_content_text(buffer))
            is_text = 1;

        if (is_encoding_gzip(buffer))
        {
            // response is compressed and should not be considered text
            is_text = 0;
            sendall(client_fd, buffer, &n);
        }
        else if (is_text)
        {
            // response is uncompressed text, save it to the text buffer.
            message_length += n;
            strcat(text_buffer, buffer);
        }
        else
        {
            // response is something else
            sendall(client_fd, buffer, &n);
        }
    }

    // Apply filter if we're dealing with uncompressed text
    if (is_text)
    {
        printf("text_buffer length is: %d\n", message_length);

        if (content_filter(text_buffer))
        {
            // Redirect.
            int rc = send(client_fd, "HTTP/1.1 302 Found\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error2.html\r\n\r\n", 96, 0);
            if (rc < 0)
            {
                fprintf(stderr, "%d: Unable to send bad content page redirect to client.\n", errno);
            }
            close(request_fd);
            return;
        }

        n = sendall(client_fd, text_buffer, &message_length);
        printf("Sent %d bytes to client.\n", message_length);
        if (n < 0)
        {
            fprintf(stderr, "%d: Unable to send text content to client.\n", errno);
            close(client_fd);
            return;
        }
    }

    close(request_fd);
    return;
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Address information for the proxy users.
    socklen_t sin_size;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;
    int rv;

    // Argument parsing for setting server port.
    char *PORT;
    if (argc == 2)
    {
        PORT = argv[1];
    }
    else if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [PORT]\n", argv[0]);
    }
    else
    {
        PORT = "3490";
    }

    // Get address information for our server.
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through the results from getaddrinfo and bind
    // to the first approrpiate socket.
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("Server: socket\n");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("Server: bind\n");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "Server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("Hey, unable to listen!");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (1)
    { // The server's main loop, waiting for new requests.
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd,
                        (struct sockaddr *)&their_addr,
                        &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork())
        { // within this 'if' is the child code.
            close(sockfd);
            handle_client_request(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }
    return 0;
}