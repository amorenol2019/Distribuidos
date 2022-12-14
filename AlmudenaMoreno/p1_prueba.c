#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#include "proxy.h"



int main(int argc, char *argv[])
{
    set_name("p1");
    set_ip_port("127.0.0.1", 8000);

    //Conectamos con el server
    client_connection();

    //Inicializamos el thread para recibir mensajes
    init_recv_thread();

    notify_ready_shutdown();
    //Esperamos a que el reloj de lamport llegue al numero correcto
    while(get_clock_lamport() < 5) {
        usleep(10000);
    }
    notify_shutdown_ack();
    printf("SHOTDOWN\n");

    sleep(1);
    close_clients();

    return 0;
}
Footer
© 2022 GitHub, Inc.
Footer navigation
Terms
Privacy
Security
Status
Docs
Contact GitHub
Pricing
API
Training
Blog
About
Sistema_distribuidos/p1.c at main · marioesteban11/Sistema_distribuidos