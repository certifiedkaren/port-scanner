#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool check_port(const char *ip, uint16_t port);

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("usage: %s <ip>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *ip = argv[1];

  FILE *fp = fopen("1000-tcp-ports.csv", "r");
  if (!fp) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  uint16_t open_ports[1000];
  int count = 0;

  char line[8192];

  while (fgets(line, sizeof(line), fp)) {
    char *token = strtok(line, ",");

    while (token != NULL) {
      uint16_t port = (uint16_t)strtol(token, NULL, 10);
      if (check_port(ip, port)) {
        open_ports[count] = port;
        count++;
        if (count >= 1000)
          break;
      };
      token = strtok(NULL, ",");
    }
  }

  if (count == 0) {
    printf("no ports found open on %s\n", ip);
  } else {
    printf("open ports on %s:\n", ip);
    for (int i = 0; i < count; i++) {
      printf("%d\n", open_ports[i]);
    }
  }

  fclose(fp);
  return 0;
}

bool check_port(const char *ip, uint16_t port) {
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));

  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);

  if (port == 0)
    return false;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return false;
  }

  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
      0) {
    close(sock);
    return false;
  }
  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) <
      0) {
    close(sock);
    return false;
  }

  int r = inet_pton(AF_INET, ip, &server.sin_addr);
  if (r != 1) {
    close(sock);
    return false;
  }

  if (connect(sock, (const struct sockaddr *)&server, sizeof(server)) == 0) {
    close(sock);
    return true;
  } else {
    // connection failed
    close(sock);
    return false;
  }
}