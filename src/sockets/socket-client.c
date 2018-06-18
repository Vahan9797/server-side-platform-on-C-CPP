//
// Created by vahan on 6/18/18.
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "../../headers/main-server.h"

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[CLIENT]: Socket Creation Failed.\n");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DEFAULT_PORT); // TODO: make this line value settable from console

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("[CLIENT]: Invalid address / Address not supported.\n");
        exit(EXIT_FAILURE);
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[CLIENT]: Connection failed.\n");
        exit(EXIT_FAILURE);
    }
    send(sock, hello, strlen(hello), 0);
    printf("[CLIENT]: Hello message sent.\n");
    valread = read(sock, buffer, sizeof(buffer));
    printf("[CLIENT]: %s\n", buffer);

    return 0;
}