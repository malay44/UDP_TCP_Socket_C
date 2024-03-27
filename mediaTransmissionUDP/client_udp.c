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
#include "constants.h"

void sendACK(int s, struct sockaddr_in sin, uint16_t sequence_no);

int main(int argc, char *argv[])
{

    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    socklen_t sin_len = sizeof(sin);
    char *host;
    char *filename;
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
        filename = argv[2];
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

    /* send fileRequest message to server */
    FileRequest fileRequest;
    fileRequest.type = FILE_REQUEST;
    fileRequest.filename_size = strlen(filename);
    strcpy(fileRequest.filename, filename);
    if (sendto(s, (FileRequest *)&fileRequest, sizeof(fileRequest), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("Client: sendto()");
        return 0;
    }

    while (1)
    {
        // receive file info and data or file not found message

        ssize_t recvLen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &sin_len);
        if (recvLen < 0)
        {
            perror("recvfrom");
            exit(1);
        }
        FileNotFound *fileNotFound;
        FileInfoAndData *fileInfoAndData;
        Data *data;

        switch (buf[0])
        {
        case FILE_NOT_FOUND:
            fileNotFound = (FileNotFound *)buf;
            printf("File not found: %s\n", fileNotFound->filename);
            close(s);
            return 0;
            break;

        case FILE_INFO_AND_DATA:
            fileInfoAndData = (FileInfoAndData *)buf;
            if (DEBUG)
            {
                printf("File info and data received.\n");
                printf("Type: %d\n", fileInfoAndData->type);
                printf("Sequence number: %d\n", (ntohs(fileInfoAndData->sequence_number)));
                printf("Filename size: %d\n", fileInfoAndData->filename_size);
                printf("Filename: %s\n", fileInfoAndData->filename);
                printf("File size: %d\n", ntohl(fileInfoAndData->file_size));
                printf("Block size: %d\n", fileInfoAndData->block_size);
                printf("Data: %s\n", fileInfoAndData->data);
            }
            sendACK(s, sin, fileInfoAndData->sequence_number);
            char filePath[255 + 15];
            strcpy(filePath, "downloads/");
            strcat(filePath, fileInfoAndData->filename);
            fp = fopen(filePath, "wb");
            if (fp == NULL)
            {
                perror("Error opening file");
                close(s);
                return 0;
            }
            fwrite(fileInfoAndData->data, 1, fileInfoAndData->block_size, fp);
            fflush(fp);
            break;

        case DATA:
            data = (Data *)buf;
            if (DEBUG)
            {
                printf("Data received.\n");
                printf("Type: %d\n", data->type);
                printf("Sequence number: %d\n", data->sequence_number);
                printf("Block size: %d\n", data->block_size);
                printf("Data: %s\n", data->data);
            }
            sendACK(s, sin, data->sequence_number);
            fwrite(data->data, 1, data->block_size, fp);
            break;
        default:
            break;
        }
    }

    // old code to send string from stdin
    fgets(buf, sizeof(buf), stdin);
    buf[BUF_SIZE - 1] = '\0';
    len = strlen(buf) + 1;
    if (sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("Client: sendto()");
        return 0;
    }

    ssize_t size_received = 0;
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

void sendACK(int s, struct sockaddr_in sin, uint16_t sequence_no)
{
    Ack ack;
    ack.type = ACK;
    ack.num_sequences = 1; // for positive ack only
    ack.sequence_no[0] = sequence_no;
    if (sendto(s, (Ack *)&ack, sizeof(ack), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("Client: sendto()");
        return;
    }
}