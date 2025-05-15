#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096

void serve_file(int client_fd, const char *path) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "public%s", strcmp(path, "/") == 0 ? "/index.html" : path);

    FILE *file = fopen(full_path, "r");
    if (!file) {
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
        write(client_fd, not_found, strlen(not_found));
        return;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *body = malloc(filesize);
    fread(body, 1, filesize, file);
    fclose(file);

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\n\r\n",
             filesize);

    write(client_fd, header, strlen(header));
    write(client_fd, body, filesize);
    free(body);
}

void start_server(int port) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    listen(server_fd, 10);
    printf("Server running on port %d...\n", port);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        int bytes = read(client_fd, buffer, BUFFER_SIZE - 1);
        buffer[bytes] = '\0';

        char method[8], path[1024];
        sscanf(buffer, "%s %s", method, path);

        printf("Request: %s %s\n", method, path);

        if (strcmp(method, "GET") == 0) {
            serve_file(client_fd, path);
        } else {
            const char *method_not_allowed = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
            write(client_fd, method_not_allowed, strlen(method_not_allowed));
        }

        close(client_fd);
    }

    close(server_fd);
}
