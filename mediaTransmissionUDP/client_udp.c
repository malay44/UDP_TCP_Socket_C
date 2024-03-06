/* CSD 304 Computer Networks, Fall 2016
   Lab 2, client
   Team:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5432
#define BUF_SIZE 4096

int main(int argc, char *argv[])
{

    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[BUF_SIZE];
    int s;
    int len;

    if ((argc == 2) || (argc == 3))
    {
        host = argv[1];
    }
    else
    {
        fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
        exit(1);
    }

    if (argc == 3)
    {
        fp = fopen(argv[2], "wb");
        if (fp == NULL)
        {
            fprintf(stderr, "Error opening output file\n");
            exit(1);
        }
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp)
    {
        fprintf(stderr, "client: unknown host: %s\n", host);
        exit(1);
    }
    else
        printf("Host %s found!\n", argv[1]);

    /* build address data structure */
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    /* create socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("client: socket");
        exit(1);
    }

    printf("Client will get data from to %s:%d.\n", argv[1], SERVER_PORT);
    printf("To play the music, pipe the download file to a player, e.g., ALSA, SOX, VLC: cat recvd_file.wav | vlc -\n");

    /* send message to server */
    fgets(buf, sizeof(buf), stdin);
    buf[BUF_SIZE - 1] = '\0';
    len = strlen(buf) + 1;
    if (sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("Client: sendto()");
        return 0;
    }

    /* get reply, display it or store in a file*/
    /* Add code to receive unlimited data and either display the data
       or if specified by the user, store it in the specified file.
       Instead of recv(), use recvfrom() call for receiving data */
    // // print message from server
    // recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len);
    // printf("Server says: %s\n", buf);
    ssize_t size_received = 0;
    socklen_t sin_len = sizeof(sin);
    while (1)
    {
        ssize_t recvLen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &sin_len);
        if (recvLen < 0)
        {
            perror("recvfrom");
            exit(1);
        }
        size_received += recvLen;
        printf("Received %ld bytes\r", size_received);
        fwrite(buf, 1, recvLen, fp);
        fflush(fp);
        if (recvLen < BUF_SIZE)
        {
            break;
        }
    }
    printf("File received of size: %ld.\n", size_received);
    fclose(fp);
    close(s);
    return 0;
}