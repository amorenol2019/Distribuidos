/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. Proxy
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//P1 y P3 a P2
//P2 recibe y envía a P1 a APAGARSE (READY_TO_SHUTDOWN)
//P1 recibe y envía ACK  (SHUTDOWN_NOW)
//P2 recibe y envía a P3 a APAGARSE (READY_TO_SHUTDOWN)
//P3 recibe y envía ACK  (SHUTDOWN_NOW)
//P2 recibe:

// Establece el nombre del proceso (para los logs y trazas)
void set_name (char name[2]) {

}

// Establecer ip y puerto
void set_ip_port (char* ip, unsigned int port) {

}

// Obtiene el valor del reloj de lamport.
// Utilízalo cada vez que necesites consultar el tiempo.
// Esta función NO puede realizar ningún tiempo de comunicación (sockets)
int get_clock_lamport() {

}

// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown() {

}

// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack() {

}

/*
MENSAJES COMUNICACION
PX, contador_lamport, SEND, operations
PX, contador_lamport, RECV (PY), operations
*/

int main (int argc, char *argv[]) {

    return 0;
}