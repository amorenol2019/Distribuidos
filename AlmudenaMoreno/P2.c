/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proxy.h"

int main (int argc, char *argv[]) {
    setbuf(stdout, NULL);

    if(argc != 3) {
        error("Incorrect program call.\n Usage: ./P2 IP_DIRECTION PORT");
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);

    set_name("P2");
    set_ip_port(ip, port);

    connect_server();

    //Recibimos los dos clientes - orden da igual
    recv_client();
    recv_client();

    //Mandamos el shutdown primero a p1 y luego p3
    //Dentro de esta funcion controlamos acks
    sending_shutdown("P1");
    sending_shutdown("P3");

    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());

    close_server();

    return 0;
}