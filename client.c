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
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("simplex-talk: socket");
    exit(1);
  }
  else
    printf("Client created socket.\n");

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }
  else
    printf("Client connected.\nClient will receive file of name: %s\n", filename);

  /* main loop: get and send lines of text and receive from message from server */
  while (fgets(buf, sizeof(buf), stdin))
  {
    buf[MAX_LINE - 1] = '\0';
    len = strlen(buf) + 1;
    send(s, buf, len, 0);
    if (strcmp(buf, "GET\n") == 0)
    {
      // send file name
      select(0, NULL, NULL, NULL, &delay);
      printf("Requesting file name: %s\n", filename);
      send(s, filename, strlen(filename) + 1, 0);
      printf("File name sent.\n");
      FILE *fp = fopen("response.txt", "w");
      if (fp == NULL)
      {
        perror("Error opening file");
        exit(1);
      }
      recv(s, buf, sizeof(buf), 0);
      file_size = atoi(buf);
      if (file_size == 0)
      {
        printf("File not found.\n");
        continue;
      }
      printf("File size: %d\n", file_size);
      int size_received = 0;
      while(file_size > 0)
      {
        recv(s, buf, sizeof(buf), 0);
        int len = strlen(buf);
        file_size -= len;
        size_received += len;
        fputs(buf, fp);
        fflush(fp);
        printf("Remaining: %ld bytes     \r", (long)file_size);
        fflush(stdout); // Flush stdout to ensure the message is printed immediately
      }
      // while ((n = read(s, buf, MAX_LINE - 1)) > 0)
      // {
      //   fputs(buf, fp);
      //   fflush(fp);
      //   file_size -= len;
      //   size_received += n;
      //   printf("Received %d bytes. Remaining: %d bytes     \r", size_received, file_size - size_received);
      //   fflush(stdout);
      //   if (buf[n - 1] == '\n')
      //   {
      //     break;
      //   }
      // }

      fclose(fp);
      printf("File size: %d\n", file_size);
      printf("File received of length %d.\n", size_received);
    }
  }
  return 0;
}

void usage(void)
{
  printf("Usage:\n");
  printf(" -p <Port>\n");
  printf(" -h <Host Address>\n"); // Corrected option description
  printf(" -f <File Name>\n");
  exit(8);
}
