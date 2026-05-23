#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BACKENDS 3

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