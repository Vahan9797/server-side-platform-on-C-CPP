//
// Created by vahan on 6/18/18.
//

#ifndef SERVER_SIDE_PLATFORM_ON_C_CPP_SERVER_H
#define SERVER_SIDE_PLATFORM_ON_C_CPP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8080
#define DEFAULT_PROTOCOL 0 // IP
#define DEFAULT_STATIC_FILES_FOLDER "../static" // execution file is in cmake-build-debug, so the static folder is level above
#define BUFSIZE 8096 // for console helper messages
#define ERROR 42
#define SORRY 43
#define LOG   44
#define DEFAULT_DB_NAME "SERVER_PLATFORM_ON_C_CPP"
#define DEFAULT_DB_DIALECT "postgres"

struct {
    char *ext;
    char *filetype;
} extensions[] = {
        {"gif", "image/gif"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"zip", "image/zip"},
        {"gz", "image/gz"},
        {"tar", "text/tar"},
        {"htm", "text/html"},
        {"html", "text/html"},
        {"php", "image/php"},
        {"cgi", "text/cgi"},
        {"asp", "text/asp"},
        {"jsp", "image/jsp"},
        {"xml", "text/xml"},
        {"js", "text/js"},
        {"css", "text/css"},
        {0,0}
};

void server_log(int, char *, char *, int);
void web(int, int);

#endif //SERVER_SIDE_PLATFORM_ON_C_CPP_SERVER_H
