#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
void start_health_checks();
#define PORT        8080
#define BUFFER_SIZE 4096

typedef struct {
    char ip[16];
    int  port;
    int  active_connections;
    int  is_alive;
} Backend;

extern Backend backends[3];
int pick_backend(int use_least_connections);

int connect_to_backend(Backend *b) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(b->port);
    addr.sin_addr.s_addr = inet_addr(b->ip);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

void forward_traffic(int client_fd, int use_lc) {
    int idx = pick_backend(use_lc);
    if (idx < 0) {
        char *err = "503 No backends available\n";
        send(client_fd, err, strlen(err), 0);
        return;
    }

    Backend *b = &backends[idx];
    int backend_fd = connect_to_backend(b);
    if (backend_fd < 0) {
        b->is_alive = 0;
        printf("Backend %d (port %d) is down\n", idx + 1, b->port);
        forward_traffic(client_fd, use_lc);
        return;
    }

    b->active_connections++;
    printf("Request → backend %d (port %d) | conns: %d\n",
           idx + 1, b->port, b->active_connections);

    char buffer[BUFFER_SIZE];
    fd_set fds;
    int max_fd = (client_fd > backend_fd ? client_fd : backend_fd) + 1;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(client_fd,  &fds);
        FD_SET(backend_fd, &fds);

        if (select(max_fd, &fds, NULL, NULL, NULL) < 0) break;

        if (FD_ISSET(client_fd, &fds)) {
            int n = read(client_fd, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            send(backend_fd, buffer, n, 0);
        }

        if (FD_ISSET(backend_fd, &fds)) {
            int n = read(backend_fd, buffer, BUFFER_SIZE);
            if (n <= 0) break;
            send(client_fd, buffer, n, 0);
        }
    }

    b->active_connections--;
    close(backend_fd);
}

int start_server(int use_lc) {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Load balancer on port %d | algo: %s\n",
        PORT, use_lc ? "least-connections" : "round-robin");
        start_health_checks();
    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr *)&address,
                               (socklen_t *)&addrlen);
        if (client_fd < 0) continue;
        forward_traffic(client_fd, use_lc);
        close(client_fd);
    }
    return 0;
}