#ifndef __NETWORKING_H__
#define __NETWORKING_H__

    int start_server(int port);
    int wait_for_connection(int server_socket);

#endif // __NETWORKING_H__