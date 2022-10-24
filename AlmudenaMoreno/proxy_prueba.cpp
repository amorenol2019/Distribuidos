
#include "proxy.h"

//Variables para el cliente y el servidor
int sockfd = 0, connfd_p1 = 0, connfd_p2 = 0, connfd_p3 = 0;
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;

struct message message;

pthread_t thread;

// Actualizamos el reloj de lamport
unsigned int lamport_increase(struct message lamport) {
    unsigned int number_lamport;
    if (lamport.clock_lamport < message.clock_lamport) {
        number_lamport = message.clock_lamport;
    }else {
        number_lamport = lamport.clock_lamport;
    }
    message.clock_lamport = number_lamport;
    message.clock_lamport++;

    return message.clock_lamport;
}

//Recibe el READY_TO_SHOTDOWN de los clientes
int wait_client_shotdown() {
    struct message receive;
    // Aceptamos el cliente
    connfd_p2 = accept(sockfd,(struct sockaddr*)NULL, NULL);
    if (connfd_p2 < 0){
        printf("Server accept failed...\n");
        exit(1);
    } else {
        printf("Server accepts the client...\n");
    }
    // Esperamos a recibir un mensaje
    if ((recv(connfd_p2, &receive, sizeof(receive),0)) < 0) 
    {
        printf("Recv from the client failed...\n");
    }else {
        // Esperamos Ready_to_shutdown
        if(receive.action == READY_TO_SHUTDOWN) 
        {
            // Reajustamos lamport
            receive.clock_lamport = lamport_increase(receive); 
            printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", message.origin, receive.clock_lamport, receive.origin);
        } else {
            printf("This is not the correct action\n");
            }   
    }
    //De que cliente es el mensaje 
    if(strcmp(receive.origin, "p1") == 0 ) {
        connfd_p1 = connfd_p2;
    }else if(strcmp(receive.origin, "p3") == 0 ) { 
        connfd_p3 = connfd_p2;
    }
    return 0;
}

int server_wait_shotdown_ack(struct message ack) {
    if (message.clock_lamport == 6) {
        connfd_p2 = connfd_p1;
    }
    else if (message.clock_lamport == 10) {
        connfd_p2 = connfd_p3;
    }
    // Esperamos al ack del cliente
    if ((recv(connfd_p2, &ack, sizeof(ack),0)) < 0) 
    {
        printf("Recv from the client failed...\n");
    } else {
        //Esperamos el shotdown ack
        if(ack.action == SHUTDOWN_ACK) {
            //  Reajustamos lamport
            ack.clock_lamport = lamport_increase(ack); 
            printf("%s, %d, RECV (%s), SHUTDOWN_ACK\n", message.origin, ack.clock_lamport, ack.origin);
        }else {
            printf("Wrong operation\n");
        }
    }
    return 0;
}

//Mandar SHOTDOWN_NOW del servidor a los clientes
int server_send_shotdown(char name[2]) {
    struct message ack;

    //QUe cliente es el que manda el mensaje
    strcpy(ack.origin, message.origin); 
    // Actualizamos lamport
    message.clock_lamport++; 
    ack.clock_lamport = message.clock_lamport;
    // Enviamos el SHOTDOWN_NOW
    ack.action = SHUTDOWN_NOW; 

    printf("%s, %d, SEND, SHUTDOWN_NOW (%s)\n",ack.origin, ack.clock_lamport, name);

    if (ack.clock_lamport == 4 ) {
        connfd_p2 = connfd_p1;
    } 
    else if (ack.clock_lamport == 8) {
        connfd_p2 = connfd_p3;
    }
    if (send(connfd_p2, &ack, sizeof(ack), 0) < 0)
    {
        printf("Send to the client failed...\n");
    }
    server_wait_shotdown_ack(ack);
}



//Iniciamos el thread para escuchar


