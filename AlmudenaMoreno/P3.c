/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P3
*/

#include "proxy.h"

#define LCq 9

int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);

    if(argc != 3) {
        error("Incorrect program call.\n Usage: ./P3 IP_DIRECTION PORT");
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);

    set_name("P3");
    set_ip_port(ip, port);

    connect_client();
    notify_ready_shutdown();

    recv_ready_shutdown("P3");
    while(get_clock_lamport() != LCq) {
        sleep(1);
    }

    notify_shutdown_ack();
    //close_client(1);

    return 0;
}
