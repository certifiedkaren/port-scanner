#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("usage: %s <ip> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sock, port;
  struct sockaddr_in server;

  memset(&server, 0, sizeof(server));
  const char *ip = argv[1];
  port = atoi(argv[2]);

  if (port <= 0 || port > 65535) {
    printf("invalid port number %d\n", port);
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    perror("setsockopt RCVTIMEO");
  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    perror("setsockopt SNDTIMEO");

  int r = inet_pton(AF_INET, ip, &server.sin_addr);
  if (r != 1) {
    if (r == 0)
      printf("invalid ip address %s\n", ip);
    else
      perror("inet_ptoi");
    close(sock);
    exit(EXIT_FAILURE);
  }

  if (connect(sock, (const struct sockaddr *)&server, sizeof(server)) == 0) {
    printf("port %d is open on %s\n", port, ip);
  } else {
    if (errno == ECONNREFUSED) {
      printf("port %d closed (connection refused) on %s\n", port, ip);
    } else {
      printf("port %d is not open on %s\n", port, ip);
    }
  }
  close(sock);
  return 0;
}
