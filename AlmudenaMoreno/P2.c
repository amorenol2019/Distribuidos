/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {
    if(argc != 3) {
        error("Incorrect program call.\n Usage: ./P2 IP_DIRECTION PORT");
    }
    char *ip_dir = argv[1];
    int port = atoi(argv[2]);

    //Conectamos con los clientes
    connect_server();

    //Recibimos los dos clientes (tiene que dar igual el orden en los que se reciba)
    recv_client();
    recv_client();

    //Mandamos el shutdown primero a p1 y luego p3
    server_send_shutdown(P1);
    recv_ack();
    server_send_shutdown(P3);
    recv_ack();

    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());

    //Una vez que ya hemos terminado todo cerramos las conexiones
    close_server();

    return 0;
}