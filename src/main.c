#include <stdio.h>
#include <string.h>

int start_server(int use_lc);

int main(int argc, char *argv[]) {
    int use_lc = 0;
    if (argc > 1 && strcmp(argv[1], "--least-connections") == 0)
        use_lc = 1;

    printf("Starting load balancer...\n");
    start_server(use_lc);
    return 0;
}