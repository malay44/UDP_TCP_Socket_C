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

#define SERVER_PORT 5433
#define MAX_PENDING 5
#define MAX_LINE 256

void usage(void);

int main(int argc, char *argv[])
{
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  socklen_t len;
  int s, new_s;
  char str[INET_ADDRSTRLEN];
  int port = SERVER_PORT;
  char filename[MAX_LINE] = "sample.txt";
  char c;
  int file_found = 1;

  while ((c = getopt(argc, argv, "p:")) != -1)
  {
    switch (c)
    {
    case 'p':
      port = atoi(optarg);
      break;
    default:
      usage();
      exit(1);
    }
  }

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("simplex-talk: socket");
    exit(1);
  }

  inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  printf(
      "Server is using address %s and port %d.\n",
      str,
      port);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
  {
    perror("simplex-talk: bind");
    exit(1);
  }
  else
    printf("Server bind done.\n");

  listen(s, MAX_PENDING);

  /* wait for connection, then receive and print text */
  while (1)
  {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0)
    {
      perror("simplex-talk: accept");
      exit(1);
    }
    printf("Server Listening.\n");
    while ((len = recv(new_s, buf, sizeof(buf), 0)))
    {
      if (strcmp(buf, "GET\n") == 0)
      {
        // get file name
        recv(new_s, filename, sizeof(filename), 0);
        printf("GET request received for %s.\n", filename);
        FILE *fp = fopen(filename, "rb");
        if (fp == NULL)
        {
          perror("Error opening file");
          file_found = 0;
        }else
        {
          file_found = 1;
        }
        if (file_found != 1)
        {
          printf("File not found.\n");
          send(new_s, "0", 2, 0);
          continue;
        }
        // get file size
        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char file_size_str[10];
        sprintf(file_size_str, "%d", file_size);
        printf("File size: %s\n", file_size_str);
        send(new_s, file_size_str, strlen(file_size_str) + 1, 0);
        usleep(1000);
        bzero(buf, sizeof(buf));
        size_t bytesRead;
        while ((bytesRead = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
          send(new_s, buf, bytesRead, 0);
        }
        fclose(fp);
      }
      bzero(buf, sizeof(buf));
    }
    close(new_s);
  }
  close(s);
  return 0;
}

void usage(void)
{
  printf("Usage:\n");
  printf(" -p <Port>\n");
  exit(8);
}