#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int start_server(int port) {
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket\n");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Bind failed\n");
        return -1;
    }

    listen(socket_desc, 3);
    return socket_desc;
}

int wait_for_connection(int server_socket) {
    int client_sock = accept(server_socket, NULL, NULL);
    if (client_sock < 0) {
        printf("Connection failed\n");
        return -1;
    }
    return client_sock;
}
