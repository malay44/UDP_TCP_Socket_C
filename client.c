#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5433
#define MAX_LINE 256

int main(int argc, char *argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s;
  int len;
  if (argc == 2)
  {
    host = argv[1];
  }
  else
  {
    fprintf(stderr, "usage: %s host\n", argv[0]);
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
    printf("Client's remote host: %s\n", argv[1]);

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);

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
    printf("Client connected.\n");

  /* main loop: get and send lines of text and recive from message from server */
  while (fgets(buf, sizeof(buf), stdin))
  {
    buf[MAX_LINE - 1] = '\0';
    len = strlen(buf) + 1;
    send(s, buf, len, 0);
    if (strncmp(buf, "GET", 3) == 0)
    {
      if (recv(s, buf, sizeof(buf), 0) == 0)
      {
        perror("Server closed connection.\n");
        close(s);
        exit(1);
      }
      fp = fopen("response.txt", "w");
      if (fp == NULL)
      {
        perror("Error opening file");
        exit(1);
      }
      fputs(buf, fp);
      fclose(fp);
    }
  }
}
