#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NUM_BACKENDS 3
#define HEALTH_INTERVAL 5

typedef struct {
    char ip[16];
    int  port;
    int  active_connections;
    int  is_alive;
} Backend;

Backend backends[NUM_BACKENDS] = {
    {"127.0.0.1", 8081, 0, 1},
    {"127.0.0.1", 8082, 0, 1},
    {"127.0.0.1", 8083, 0, 1},
};

static int rr_index = 0;

int pick_round_robin() {
    int tries = 0;
    while (tries < NUM_BACKENDS) {
        int i = rr_index % NUM_BACKENDS;
        rr_index++;
        if (backends[i].is_alive) return i;
        tries++;
    }
    return -1;
}

int pick_least_connections() {
    int best = -1;
    for (int i = 0; i < NUM_BACKENDS; i++) {
        if (!backends[i].is_alive) continue;
        if (best == -1 || backends[i].active_connections < backends[best].active_connections)
            best = i;
    }
    return best;
}

int pick_backend(int use_least_connections) {
    return use_least_connections ? pick_least_connections() : pick_round_robin();
}

int check_backend_health(Backend *b) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(b->port);
    addr.sin_addr.s_addr = inet_addr(b->ip);

    struct timeval timeout = {2, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    int result = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);
    return result == 0;
}

void *health_check_loop(void *arg) {
    while (1) {
        sleep(HEALTH_INTERVAL);
        for (int i = 0; i < NUM_BACKENDS; i++) {
            int alive = check_backend_health(&backends[i]);
            if (alive && !backends[i].is_alive) {
                printf("[health] backend %d (port %d) is back UP\n",
                       i + 1, backends[i].port);
            } else if (!alive && backends[i].is_alive) {
                printf("[health] backend %d (port %d) went DOWN\n",
                       i + 1, backends[i].port);
            }
            backends[i].is_alive = alive;
        }
    }
    return NULL;
}

void start_health_checks() {
    pthread_t thread;
    pthread_create(&thread, NULL, health_check_loop, NULL);
    pthread_detach(thread);
    printf("Health checks started (every %ds)\n", HEALTH_INTERVAL);
}