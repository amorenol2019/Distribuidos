/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. Proxy
*/

#include "proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int sockfd = 0, connfd_p1 = 0, connfd_p2 = 0, connfd_p3 = 0;
struct sockaddr_in sock_serv;
struct sockaddr_in sock_cli;

struct message msg;

pthread_t thread;

void error(char *msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

// Establece el nombre del proceso (para los logs y trazas)
void set_name (char name[2]) {
    strcpy(msg.origin, name);
}

// Establecer ip y puerto
void set_ip_port (char* ip, unsigned int port) {
    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);

    bzero(&sock_cli, sizeof(sock_cli));
    sock_cli.sin_family = AF_INET;
    sock_cli.sin_addr.s_addr = inet_addr(ip);
    sock_cli.sin_port = htons(port);
}

// Obtiene el valor del reloj de lamport.
// Utilízalo cada vez que necesites consultar el tiempo.
// Esta función NO puede realizar ningún tiempo de comunicación (sockets)
int get_clock_lamport() {
    return msg.clock_lamport;
}

// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown() {
    struct message msg2
    strcpy(msg2.origin, msg.origin)
    msg.clock_lamport++;
    msg2.clock_lamport = msg.clock_lamport;
    msg2.action = READY_TO_SHUTDOWN;

    printf("%s, %d, SEND, READY_TO_SHUTDOWN\n", msg2.origin, msg2.clock_lamport);

    if (send(sockfd, &msg2, sizeof(ready), 0) == -1) {
        printf("Send to server failed...\n");
    }
}

// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack() {
    struct message msg2
    strcpy(msg2.origin, msg.origin)
    msg.clock_lamport++;
    msg2.clock_lamport = msg.clock_lamport;
    msg2.action = SHUTDOWN_ACK;
    printf("%s, %d, SEND, SHUTDOWN_ACK\n", msg2.origin, msg2.clock_lamport);

    if (send(sockfd, &msg2, sizeof(ready), 0) == -1) {
        printf("Send to server failed...\n");
    }
}

int connect_client() {
    /*Create a socket and test*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }

    if((connect(sockfd, (struct sockaddr*)&sock_cli, sizeof(sock_cli))) == -1){
        close(sockfd);
        error("Connection with the server failed...");
    } else{
        printf("Connected to the server...\n");
    }
    return 0;
}

int connect_server() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }
    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1){
        error("Socket bind failed...");
    } else {
        printf("Socket successfully binded...\n");
    }
    if((listen(sockfd, 2)) == -1){
        error("Listen failed...");
    } else {
        printf("Server listening...\n");
    }
    return 0;
}

int close_server() {
    if(close(connfd_p2) == -1) {
        error("Close failed");
    }
    if(close(sockfd) == -1) {
        error("Close failed");
    }
  return 0;
}

int close_client(){
    if(pthread_join(thread, NULL) == -1) {
      error("join failed");
    }
    if(close(connfd_p2) == -1) {
        error("Close failed");
    }
    if(close(sockfd) == -1) {
        error("Close failed");
    }

    return 0;
}

void init_thread () {
    pthread_create(&thread, NULL, msg_shutdown, NULL);
}

void *msg_shutdown() {
    while(1) {
        struct message shutdown;
        usleep(5000);
        if ((recv(sockfd, &shutdown, sizeof(shutdown),MSG_DONTWAIT)) > 0) {
            if(shutdown.action == SHUTDOWN_NOW) {
                shutdown.clock_lamport = act_lamport(shutdown);
                printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", message.origin, shutdown.clock_lamport, shutdown.origin);
                break;
            } else {
                printf("ERROR : Not a valid operation\n");
            }
        }

    }

}

// Actualizamos el reloj
unsigned int act_lamport(struct message clock) {
    unsigned int counter;
    if (clock.clock_lamport < msg.clock_lamport) {
        counter = msg.clock_lamport;
    }else {
        counter = clock.clock_lamport;
    }
    msg.clock_lamport = counter;
    msg.clock_lamport++;

    return msg.clock_lamport;
}

int recv_client() {
    struct message recv;

    connfd_p2 = accept(sockfd,(struct sockaddr*)NULL, NULL);
    if (connfd_p2 < 0){
        error("Failed server accept...");
    }

    if ((recv(connfd_p2, &recv, sizeof(recv),0)) == -1) {
        printf("Receive data from client failed...\n");
    }else {
        // READY_TO_SHUTDOWN
        if(recv.action == READY_TO_SHUTDOWN) {
            // Actualizamos reloj
            recv.clock_lamport = act_lamport(recv); 
            printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", msg.origin, recv.clock_lamport, recv.origin);
        } else {
            printf("ERROR : Not a valid operation\n");
        }   
    }

    //CLIENTE P1 O P3
    if(strcmp(recv.origin, "P1") == 0 ) {
        connfd_p1 = connfd_p2;
    }else if(strcmp(recv.origin, "P3") == 0 ) { 
        connfd_p3 = connfd_p2;
    }

    return 0;
}

int recv_ack(struct message ack) {
    if (message.clock_lamport == 6) {
        connfd_p2 = connfd_p1;
    }
    else if (message.clock_lamport == 10) {
        connfd_p2 = connfd_p3;
    }
    // Esperamos al ack del cliente
    if ((recv(connfd_p2, &ack, sizeof(ack),0)) == -1) {
        printf("Receive data from client failed...\n");
    } else {
        //Esperamos el shotdown ack
        if(ack.action == SHUTDOWN_ACK) {
            ack.clock_lamport = act_lamport(ack); 
            printf("%s, %d, RECV (%s), SHUTDOWN_ACK\n", msg.origin, ack.clock_lamport, ack.origin);
        }else {
            printf("ERROR : Not a valid operation\n");
        }
    }

    return 0;
}

int sending_shutdown(char name[2]) {
    struct message ack;
    strcpy(ack.origin, msg.origin); 

    msg.clock_lamport++; 
    ack.clock_lamport = msg.clock_lamport;
    ack.action = SHUTDOWN_NOW; 

    printf("%s, %d, SEND, SHUTDOWN_NOW (%s)\n",ack.origin, ack.clock_lamport, name);

    if (ack.clock_lamport == 4 ) {
        connfd_p2 = connfd_p1;
    } 
    else if (ack.clock_lamport == 8) {
        connfd_p2 = connfd_p3;
    }

    if (send(connfd_p2, &ack, sizeof(ack), 0) == -1) {
        printf("ERROR sending the message\n");
    }

    recv_ack(ack);
}

/*


*/

//P1 y P3 a P2
//P2 recibe y envía a P1 a APAGARSE (READY_TO_SHUTDOWN)
//P1 recibe y envía ACK  (SHUTDOWN_NOW)
//P2 recibe y envía a P3 a APAGARSE (READY_TO_SHUTDOWN)
//P3 recibe y envía ACK  (SHUTDOWN_NOW)
//P2 recibe:
/*
MENSAJES COMUNICACION
PX, contador_lamport, SEND, operations
PX, contador_lamport, RECV (PY), operations
*/

int main (int argc, char *argv[]) {

    return 0;
}