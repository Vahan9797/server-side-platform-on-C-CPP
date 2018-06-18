//
// Created by vahan on 6/18/18.
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "../../headers/main-server.h"

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // Creating socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL)) == 0) {
        perror("[SERVER]: Socket Failed.\n");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port given by console or DEFAULT_PORT
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("[SERVER]: Setsockopt Failed.\n");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(DEFAULT_PORT); // TODO: make this line value settable from console

    // Forcefully attaching socket to the port given by console or DEFAULT_PORT
    if(bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("[SERVER]: Bind Failed.\n");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 3) < 0) {
        perror("[SERVER]: Listen Failed.\n");
        exit(EXIT_FAILURE);
    }

    if((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("[SERVER]: Accept Failed.\n");
        exit(EXIT_FAILURE);
    }

    valread = read(new_socket, buffer, sizeof(buffer));
    printf("[SERVER]: %s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("[SERVER]: Hello message sent\n");

    return 0;
}