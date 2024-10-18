#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ui.h"

int connect_to_server(const char *server_ip, int server_port) {
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket\n");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect failed. Error\n");
        return -1;
    }

    return socket_desc;
}


