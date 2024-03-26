/* CSD 304 Computer Networks, Fall 2016
   Lab 2, server
   Team:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "constants.h"

typedef struct
{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
} ThreadArgs;

void *handle_client(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    struct sockaddr_storage client_addr = args->client_addr;
    socklen_t client_addr_len = args->client_addr_len;

    char clientIP[INET_ADDRSTRLEN];
    int client_socket;
    char buf[BUF_SIZE];

    inet_ntop(client_addr.ss_family,
              &(((struct sockaddr_in *)&client_addr)->sin_addr),
              clientIP, INET_ADDRSTRLEN);

    printf("Server %d: Got connection from %s\n", pthread_self(), clientIP);
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("server: socket");
        exit(1);
    }
    printf("Server %d: New Socket Created for client %s.\n", pthread_self(), clientIP);

    printf("Server %d: Sending file to client\n", pthread_self());
    size_t bytes_read;
    FILE *fp = fopen("streamable.mp4", "rb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    ssize_t size_sent = 0;
    while ((bytes_read = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        usleep(300);
        sendto(client_socket, buf, bytes_read, 0, (struct sockaddr *)&client_addr, client_addr_len);
        size_sent += bytes_read;
        printf("Server %d: Sent %ld bytes\r", pthread_self(), size_sent);
    }
    printf("Server %d: File sent of size: %ld\n", pthread_self(), size_sent);
    fclose(fp);
    close(client_socket);
    memset(buf, 0, sizeof(buf));
}

int main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
    char buf[BUF_SIZE];
    int len;
    int s;
    char *host;
    struct hostent *hp;

    printf("Server process id: %d\n", getpid());

    /* Create a socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("server: socket");
        exit(1);
    }

    /* build address data structure and bind to all local addresses*/
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    /* If socket IP address specified, bind to it. */
    if (argc == 2)
    {
        host = argv[1];
        hp = gethostbyname(host);
        if (!hp)
        {
            fprintf(stderr, "server: unknown host %s\n", host);
            exit(1);
        }
        memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    }
    /* Else bind to 0.0.0.0 */
    else
        sin.sin_addr.s_addr = INADDR_ANY;

    sin.sin_port = htons(SERVER_PORT);

    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
    {
        perror("server: bind");
        exit(1);
    }
    else
    {
        // Parse the binary IP address to a human-readable form
        inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN); // clientIP var is used to store the IP address of the server over here.
        printf("Server is listening at address %s:%d\n", clientIP, SERVER_PORT);
    }

    printf("Client needs to send \"GET\" to receive the file %s\n", argv[1]);

    while (1)
    {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        memset(buf, 0, sizeof(buf));
        /* Receive messages from clients*/
        printf("Server Listening.\n");
        if ((len = recvfrom(s, buf, sizeof(buf), 0,
                            (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("Main Server: recvfrom");
            exit(1);
        }
        FileRequest *fileRequest = (FileRequest *)buf;

        if (DEBUG)
        {
            printf("Server: Received %d bytes\n", len);
            printf("Server: size of FileRequest: %ld\n", sizeof(FileRequest));
            printf("Server: fileRequest->type: %hhu\n", fileRequest->type);
            printf("Server: fileRequest->filename_size: %hhu\n", fileRequest->filename_size);
            printf("Server: fileRequest->filename: %s\n", fileRequest->filename);
        }

        // old code
        if (strcmp(buf, "GET\n") == 0)
        {
            int s = socket(AF_INET, SOCK_DGRAM, 0);
            ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            args->client_addr = client_addr;
            args->client_addr_len = client_addr_len;
            pthread_t tid;
            if (pthread_create(&tid, NULL, handle_client, (void *)args) != 0)
            {
                perror("pthread_create");
                exit(1);
            }
            pthread_detach(tid);
        }
        else
        {
            printf("Server: Invalid request\n");
        }
        memset(buf, 0, sizeof(buf));
    }
}
