//
// Created by vahan on 6/18/18.
//

// Simple HTTP Server. Currently supports only simple GET requests.
#include "headers/server.h"

int main(int argc, char **argv) {
    int i, port, pid, listenfd, socketfd;
    size_t length;
    static struct sockaddr_in cli_addr;
    static struct sockaddr_in serv_addr;

    if(argc < 3 || argc > 3 || !strcmp(argv[1], "-?")) {
        (void)printf("usage: server [port] [server directory] &\t Example: server 80\n\n");
        (void)printf("\nOnly Supports:\n");

        for(i = 0; extensions[i].ext != 0; i++) {
            (void)printf(" %s", extensions[i].ext);
        }

        (void)printf("\n\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin\n");
        exit(0);
    }

    if(!strncmp(argv[2], "/",    2) || !strncmp(argv[2], "/etc", 5) ||
       !strncmp(argv[2], "/bin", 5) || !strncmp(argv[2], "/lib", 5) ||
       !strncmp(argv[2], "/tmp", 5) || !strncmp(argv[2], "/usr", 5) ||
       !strncmp(argv[2], "/dev", 5) || !strncmp(argv[2], "/sbin", 6)
      ) {
        (void)printf("ERROR: Bad top directory %s, see server -?\n", argv[2]);
        exit(3);
    }

    if(chdir(argv[2]) == -1) {
        (void)printf("ERROR: Can't change to directory %s\n", argv[2]);
        exit(4);
    }

    if(fork() != 0) {
        return 0;
    }

    (void)signal(SIGCLD, SIG_IGN);
    (void)signal(SIGHUP, SIG_IGN);

    for(i = 0; i < 32; i++) {
        (void)close(i);
    }

    (void)setpgrp();

    server_log(LOG, "HTTP server starting", argv[1], getpid());

    if((listenfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL)) < 0) {
        server_log(ERROR, "System call", "socket", 0);
    }

    port = atoi(argv[1]) || DEFAULT_PORT;

    if(port < 0 || port > 60000) {
        server_log(ERROR, "Invalid port number try [1, 60000]", argv[1], 0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons((uint16_t) port);

    if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        server_log(ERROR, "System call", "bind", 0);
    }

    if(listen(listenfd, 64) < 0) {
        server_log(ERROR, "System call", "listen", 0);
    }

    for(int hit = 1;; hit++) {
        length = sizeof(cli_addr);

        if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t *) length)) < 0) {
            server_log(ERROR, "System call", "accept", 0);
        }

        if((pid = fork()) < 0) {
            server_log(ERROR, "System call", "fork", 0);
        } else {
            if(pid == 0) {
                (void)close(listenfd);
                web(socketfd, hit);
            } else {
                (void)close(socketfd);
            }
        }
    }
}

void server_log(int type, char *s1, char *s2, int num) {
    int fd;
    char logBuffer[BUFSIZE*2];

    switch(type) {
        case ERROR:
            (void)sprintf(logBuffer, "ERROR: %s %s Errno=%d exiting pid=%d", s1, s2, errno, getpid());
            break;
        case SORRY:
            (void)sprintf(logBuffer, "<HTML><BODY><H1>Web server sorry: %s %s </H1></BODY></HTML>\r\n", s1, s2);
            (void)write(num, logBuffer, strlen(logBuffer));
            (void)sprintf(logBuffer, "SORRY: %s:%s", s1, s2);
            break;
        default:
        case LOG:
            (void)sprintf(logBuffer, "INFO: %s:%s:%d", s1, s2, num);
            break;
    }

    if((fd = open("log/server.log", O_CREAT| O_WRONLY | O_APPEND, 0644)) >= 0) {
        (void)write(fd, logBuffer, strlen(logBuffer));
        (void)write(fd, "\n", 1);
        (void)close(fd);
    }

    if(type == ERROR || type == SORRY) exit(3);
}

void web(int fd, int hit) {
    int file_fd;
    long i, ret;
    char *fstr;
    size_t buflen, len;
    static char buffer[BUFSIZE + 1];

    ret = read(fd, buffer, BUFSIZE);

    if(ret == 0 || ret == -1) {
        server_log(SORRY, "Failed to read browser request", "", fd);
    }

    if(ret > 0 && ret < BUFSIZE) {
        buffer[ret] = 0;
    } else {
        buffer[0] = 0;
    }

    for(i = 0; i < ret; i++) {
        if(buffer[i] == '\r' || buffer[i] == '\n') {
            buffer[i] = '*';
        }
    }

    server_log(LOG, "Request", buffer, hit);

    if(strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4)) {
        server_log(SORRY, "Only simple GET operation supported for now.", buffer, fd);
    }

    for(i = 4; i < BUFSIZE; i++) {
        if(buffer[i] == ' ') {
            buffer[i] = 0;
            break;
        }
    }

    for(int j = 0; j < i - 1; j++) {
        if(buffer[j] == '.' && buffer[j+1] == '.') {
            server_log(SORRY, "Parent directory (..) path names not supported.", buffer, fd);
        }
    }

    if(!strncmp(&buffer[0], "GET /\0", 6) || !strncmp(&buffer[0], "get /\0",6)) {
        (void)strcpy(buffer, "GET /index.html");
    }

    buflen = strlen(buffer);
    fstr = (char *)0;
    for(i = 0; extensions[i].ext != 0; i++) {
        len = strlen(extensions[i].filetype);
        if(!strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
            fstr = extensions[i].filetype;
            break;
        }
    }

    if(fstr == 0) {
        server_log(SORRY, "File Extension type not supported.", buffer, fd);
    }

    if((file_fd = open(&buffer[5], O_RDONLY)) == -1) {
        server_log(SORRY, "Failed to open file.", &buffer[5], fd);
    }

    server_log(LOG, "SEND", &buffer[5], hit);

    (void)sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-type: %s\r\n\r\n", fstr);
    (void)write(fd, buffer, strlen(buffer));

    while((ret = read(file_fd, buffer, BUFSIZE)) > 0) {
        (void)write(fd, buffer, ret);
    }
    #ifdef LINUX
        sleep(1);
    #endif
        exit(1);
}