#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 3000
#define MAX_LINE 256

void usage(void);

int main(int argc, char *argv[])
{
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host = NULL;
  char buf[MAX_LINE];
  int s;
  int len;
  int server_port = SERVER_PORT;
  char c;
  int file_size = 0;
  int n;
  char filename[MAX_LINE] = "sample.txt";
  struct timeval delay;
  delay.tv_sec = 0; // Delay in seconds
  delay.tv_usec = 300;

  while ((c = getopt(argc, argv, "p:h:f:")) != -1)
  {
    switch (c)
    {
    case 'p':
      server_port = atoi(optarg);
      break;
    case 'h':
      host = optarg;
      break;
    case 'f':
      strcpy(filename, optarg);
      break;
    default:
      usage();
      exit(1);
    }
  }

  if (host == NULL)
  {
    usage();
    exit(1);
  }

  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp)
  {
    fprintf(stderr, "%s: unknown host: %s\n", argv[0], host);
    exit(1);
  }
  else
    printf("Client's remote host: %s\n", host);

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(server_port);

  /* active open */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("simplex-talk: socket");
    exit(1);
  }
  else
    printf("Client created socket.\n");

  /* Requesting the file */
  sendto(s, "GET\n\0", 5, 0, (struct sockaddr *)&sin, sizeof(sin));
  usleep(100);
  printf("Requesting file: %s\n", filename);
  sendto(s, filename, strlen(filename) + 1, 0, (struct sockaddr *)&sin, sizeof(sin));
  printf("File name sent.\n");

  FILE *fp = fopen("response.txt", "wb");
  if (fp == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

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

    if (strcmp(buf, "0") == 0)
    {
      printf("File not found.\n");
      exit(1);
    }
    else if (strcmp(buf, "0") != 0)
    {
      // File size received
      file_size = atoi(buf);
      printf("File size: %d\n", file_size);
      break;
    }
  }

  while (file_size > 0)
  {
    ssize_t recvLen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &sin_len);
    if (recvLen < 0)
    {
      perror("recvfrom");
      exit(1);
    }

    file_size -= recvLen;
    size_received += recvLen;
    fwrite(buf, 1, recvLen, fp);
    fflush(fp);
    printf("Remaining: %ld bytes     \r", (long)file_size);
    fflush(stdout); // Flush stdout to ensure the message is printed immediately
  }

  fclose(fp);
  printf("File received of length %zd.\n", size_received);
}

void usage(void)
{
  printf("Usage:\n");
  printf(" -p <Port>\n");
  printf(" -h <Host Address>\n");
  printf(" -f <File Name>\n");
  printf("Example: gcc client.c -o client && ./client -h 0.0.0.0 -p 3000 -f sample.txt\n");
  exit(8);
}
