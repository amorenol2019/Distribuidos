/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LCq 5

int main (int argc, char *argv[]) {
    //Primero de todo es abrir canales de comunicación
    if(argc != 3) {
        error("Incorrect program call.\n Usage: ./P1 IP_DIRECTION PORT");
    }
    char *ip_dir = argv[1];
    int port = argv[2];

    connect_client();

    init_thread();

    notify_ready_shutdown;

    while(get_clock_lamport() < LCq) {
        usleep(10000)
    }

    notify_shutdown_ack();
    printf("SHUTDOWN\n");

    close_client();

    return 0;
}