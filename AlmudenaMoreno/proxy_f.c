/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. Proxy
*/

#include "proxy.h"

#define MAX 20

int sockfd = 0, id = 0, counter = 0, p1, p3;
struct sockaddr_in sock;
struct sockaddr_in sock_serv;
int connfd[2];
struct message msg;

pthread_t thread[3];

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

// Establece el nombre del proceso (para los logs y trazas)
void set_name (char name[2]) {
    strcpy(msg.origin, name);
    msg.clock_lamport = 0;
}

// Establecer ip y puerto
void set_ip_port (char* ip, unsigned int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }
    
    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
    
    bzero(&sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(ip);
    sock.sin_port = htons(port);
}

// Obtiene el valor del reloj de lamport.
// Utilízalo cada vez que necesites consultar el tiempo.
// Esta función NO puede realizar ningún tiempo de comunicación (sockets)
int get_clock_lamport() {
    return msg.clock_lamport;
}

// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown() {
    struct message msg2;
    strcpy(msg2.origin, msg.origin);
    msg.clock_lamport++;
    msg2.clock_lamport = msg.clock_lamport;
    msg2.action = READY_TO_SHUTDOWN;

    printf("%s, %d, SEND, READY_TO_SHUTDOWN\n", msg2.origin, msg2.clock_lamport);

    if (send(sockfd, &msg2, sizeof(msg2), 0) == -1) {
        printf("Send to server failed...\n");
    }
}

// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack() {
    struct message msg2;
    strcpy(msg2.origin, msg.origin);
    msg.clock_lamport++;
    msg2.clock_lamport = msg.clock_lamport;
    msg2.action = SHUTDOWN_ACK;
    printf("%s, %d, SEND, SHUTDOWN_ACK\n", msg2.origin, msg2.clock_lamport);

    if (send(sockfd, &msg2, sizeof(msg2), 0) == -1) {
        printf("Send to server failed...\n");
    }
}


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

//INCIAR CONEXION SERVIDOR
int connect_server() {
    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1){
        //close_server();
        error("Socket bind failed...");
    } else {
        printf("Socket successfully binded...\n");
    }
    if((listen(sockfd, 2)) == -1){
        //close_server();
        error("Listen failed...");
    } else {
        printf("Server listening...\n");
    }
    return 0;
}

//CONECTAR CLIENTE CON SERVIDOR
//2 CLIENTES, 2 HILOS
int connect_client() {
    /*Create a socket and test*/
    if((connect(sockfd, (struct sockaddr*)&sock, sizeof(sock))) == -1){
        close(sockfd);
        error("Connection with the server failed...");
    } else{
        printf("Connected to the server...\n");
    }
    return 0;
}

unsigned int act_lamport(struct message clock) {
    if (clock.clock_lamport < msg.clock_lamport) {
        return msg.clock_lamport + 1;
    }
    else {
        return clock.clock_lamport + 1;
    }
}

//SERVIDOR ACEPTA CLIENTES
void recv_client() {
    pthread_create(&thread[2], NULL, &msg_ready, NULL);
    if(pthread_join(thread[2], NULL) == -1) {
        //close_server();
        error("join failed");
    }
}

void *msg_ready() {
    while(counter < 2) {
        struct message msg_recv;
        connfd[id] = accept(sockfd,(struct sockaddr*)NULL, NULL);

        if (connfd[id] == -1){
            //close(connfd[id]);
            //close_server();
            error("Failed server accept...");
        }
        if ((recv(connfd[id], &msg_recv, sizeof(msg_recv),0)) == -1) {
            printf("Receive data from client failed...\n");
        }else {
            // READY_TO_SHUTDOWN
            if(msg_recv.action == READY_TO_SHUTDOWN) {
                // Actualizamos reloj
                msg.clock_lamport = act_lamport(msg_recv); 
                printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", msg.origin, msg.clock_lamport, msg_recv.origin);
            } else {
                printf("ERROR : Not a valid operation\n");
            }   
        }
        
        //CLIENTE P1 O P3
        if( strcmp("P1", msg_recv.origin) == 0 ) {
            p1 = id;
        } else {
            p3 = id;
        }
        id++;
        counter++;
    }
}

int notify_shutdown_now(char name[2]) {
    struct message ack;
    strcpy(ack.origin, msg.origin); 
    int index;
    msg.clock_lamport++;
    ack.clock_lamport = msg.clock_lamport;
    ack.action = SHUTDOWN_NOW; 

    printf("%s, %d, SEND, SHUTDOWN_NOW (%s)\n",ack.origin, ack.clock_lamport, name);

    if (ack.clock_lamport == 4 && strcmp(name, "P1") == 0) {
        index = p1;
    } else if (ack.clock_lamport == 8 && strcmp(name, "P3") == 0) {
        index = p3;
    }
    else {
        //close_server();
        error("Clock does not correspond to expected");
    }

    if (send(connfd[index], &ack, sizeof(ack), 0) == -1) {
        printf("ERROR sending the message\n");
    }
    printf("CLOCK P2 %d\n", get_clock_lamport());

    return 0;
}

//INICIA EL THREAD DE UN CLIENTE
void recv_ready_shutdown(char id_client[2]) {
    if (strcmp(id_client, "P1") == 0) {
        pthread_create(&thread[p1], NULL, msg_shutdown, &p1);
    } else if (strcmp(id_client, "P3") == 0) {
        pthread_create(&thread[p3], NULL, msg_shutdown, &p3);
    } else {
        error(("INIT THREAD %s Not valid client" , id_client));
    }
}

//THREAD CLIENTE recibe SHUTDOWN_NOW, si no, error.
void *msg_shutdown(void *idc) {
    int client = *(int *) idc;
    while ( msg.clock_lamport != 4 && msg.clock_lamport != 8) {
        struct message shutdown;

        if ((recv(sockfd, &shutdown, sizeof(shutdown), 0)) != -1) {
            if(shutdown.action == SHUTDOWN_NOW) {
                msg.clock_lamport = act_lamport(shutdown);
                if (client == p1 && msg.clock_lamport == 5) {
                    printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", msg.origin, msg.clock_lamport, shutdown.origin);
                } else if (client == p3 && msg.clock_lamport == 9) {
                    printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", msg.origin, msg.clock_lamport, shutdown.origin);
                } else {
                    printf("%s%d\n",shutdown.origin, shutdown.clock_lamport);
                    error("Clock does not correspond to expected");
                }
            } else {
                printf("ACTION %d\n", shutdown.action);
                //close_server();
                error("ERROR : Not a valid operation");
            }
        }
        else {
            //close_server();
            error("Receive data from client failed...");
        }

    }
}

int recv_ack(char name[2]) {

    int index;
    struct message ack;

    if (strcmp(name, "P1") == 0) {
        index = p1;
    }
    else if (strcmp(name, "P3") == 0) {
        index = p3;
    }
    
    // Esperamos al ack del cliente
    if (recv(connfd[index], &ack, sizeof(ack),0) == -1) {
        printf("Receive data from client failed...\n");
    } else {
        //Esperamos el shotdown ack
        if(ack.action == SHUTDOWN_ACK) {
            msg.clock_lamport = act_lamport(ack); 
            printf("%s, %d, RECV (%s), SHUTDOWN_ACK\n", msg.origin, msg.clock_lamport, ack.origin);
        }else {
            //close_server();
            error("RECV_ACK ERROR : Not a valid operation");
        }
    }

    return 0;
}



