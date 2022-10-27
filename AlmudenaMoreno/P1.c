/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P1
*/

#include "proxy.h"

#define LCq 5

int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);

    if(argc != 3) {
        error("Incorrect program call.\n Usage: ./P1 IP_DIRECTION PORT");
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);

    set_name("P1");
    set_ip_port(ip, port);

    connect_client();

    init_thread();

    notify_ready_shutdown();

    while(get_clock_lamport() != LCq) {
        sleep(1);
    }

    notify_shutdown_ack();
    printf("SHUTDOWN ACTIVE\n");

    close_client();

    return 0;
}