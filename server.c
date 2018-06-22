//
// Created by vahan on 6/18/18.
//

// Simple HTTP Server. Currently supports only simple GET requests.
#include "headers/server.h"

int main(int argc, char **argv) {
    char static_dir[PATH_MAX + 1];
    int i, pid, listenfd, socketfd, hit;
    long port = DEFAULT_PORT;
    size_t length;
    static struct sockaddr_in cli_addr;
    static struct sockaddr_in serv_addr;

    if(argc != 1 && (argc < 3 || argc > 3 || !strcmp(argv[1], "-?"))) {
        (void)printf("usage: server [port] [server directory] &\t Example: server 80\n\n");
        (void)printf("\nOnly Supports:");

        for(i = 0; extensions[i].ext != 0; i++) {
            (void)printf(" %s", extensions[i].ext);
        }

        (void)printf("\n\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin\n");
        exit(0);
    }

    if(argv[2] && strstr(argv[2], "PATH") == NULL) {
        sprintf(static_dir, "%s", argv[2]);
    } else {
        realpath(DEFAULT_STATIC_FILES_FOLDER, static_dir);
    }

    if(!strncmp(static_dir, "/",    2) || !strncmp(static_dir, "/etc", 5) ||
       !strncmp(static_dir, "/bin", 5) || !strncmp(static_dir, "/lib", 5) ||
       !strncmp(static_dir, "/tmp", 5) || !strncmp(static_dir, "/usr", 5) ||
       !strncmp(static_dir, "/dev", 5) || !strncmp(static_dir, "/sbin", 6)
      ) {
        (void)printf("ERROR: Bad top directory %s, see server -?\n", static_dir);
        exit(3);
    }

    if(chdir(static_dir) == -1) {
        (void)printf("ERROR: Can't change to directory %s\n", static_dir);
        exit(4);
    }

    if(fork() == 0) {
        (void)signal(SIGCLD, SIG_IGN);
        (void)signal(SIGHUP, SIG_IGN);

        for(i = 0; i < 32; i++) {
            (void)close(i);
        }

        (void)setpgrp();

        if(argv[1] != NULL && strstr(argv[1], "PATH") == NULL) {
            port = strtol(argv[1], NULL, 10);
        }

        char portParse[5];
        sprintf(portParse, "%lu", port); // converting port to string for segmentation fault error

        server_log(LOG, "HTTP server starting", portParse, getpid());

        if((listenfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL)) < 0) {
            server_log(ERROR, "System call", "socket", 0);
        }

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

        for(hit = 1;; hit++) {
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
}

void server_log(int type, char *s1, char *s2, int num) {
    int fd;
    char logBuffer[BUFSIZE*2] = {0};

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
    #ifdef UNIX
        sleep(1);
    #endif
        exit(1);
}