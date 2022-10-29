/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. P2
*/

#include "proxy.h"
#include <signal.h>

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
    signal(SIGINT, ctrlHandlerServer);   //Close with CTRL + C

    recv_client();
    recv_client();

    notify_shutdown_now("P1");
    recv_ack("P1");
    while(get_clock_lamport() != 7) {    //Hasta que clock no es valido para seguir despues del ack
        sleep(1);
    }

    notify_shutdown_now("P3");
    recv_ack("P3");
    while(get_clock_lamport() != 11) {    //Hasta que clock no es valido para seguir despues del ack
        sleep(1);
    }

    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());
    close_server();

    return 0;
}