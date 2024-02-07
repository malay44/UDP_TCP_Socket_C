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

int main() {
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  socklen_t len;
  int s, new_s;
  char str[INET_ADDRSTRLEN];

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);
  
  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }
 
  inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  printf("Server is using address %s and port %d.\n", str, SERVER_PORT);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
  }
  else
    printf("Server bind done.\n");

  listen(s, MAX_PENDING);
  
  /* wait for connection, then receive and print text */
  while(1) {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }
    printf("Server Listening.\n");
    while ((len = recv(new_s, buf, sizeof(buf), 0))){
      if (strncmp(buf, "GET", 3) == 0) {
        printf("GET request received.\n");
        FILE *fp = fopen("sample.txt", "r");
        if (fp == NULL) {
          perror("Error opening file");
          send(new_s, "Not Found", 9, 0);
        }
        char content[MAX_LINE];
        while (fgets(content, sizeof(content), fp) != NULL) {
          send(new_s, content, strlen(content), 0);
        }
      }
      fputs(buf, stdout);
    }
    close(new_s);
  }
  close(s);
  return 0;
}
