/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. Proxy
*/

#include "proxy.h"

#define MAX 20
#define MAX_CLIENTS 2

int sockfd = 0, i = 0;
struct sockaddr_in sock;
struct sockaddr_in sock_serv;
int connfd[MAX_CLIENTS];
char p1[MAX], p3[MAX];
struct message msg;

pthread_t thread;

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

int max(int a, int b) {
    if (a < b) {
        return b;
    }
    else {
        return a;
    }
}

unsigned int act_lamport(struct message clock) {
    return max(clock.clock_lamport, msg.clock_lamport) + 1;
}

//SERVIDOR ACEPTA CLIENTES
int recv_client() {
    struct message msg_recv;

    connfd[i] = accept(sockfd,(struct sockaddr*)NULL, NULL);
    if (connfd[i] == -1){
        close(connfd[i]);
        error("Failed server accept...");
    }

    if ((recv(connfd[i], &msg_recv, sizeof(msg_recv),0)) == -1) {
        printf("Receive data from client failed...\n");
    }else {
        // READY_TO_SHUTDOWN
        if(msg_recv.action == READY_TO_SHUTDOWN) {
            // Actualizamos reloj
            msg.clock_lamport = act_lamport(msg_recv); 
            msg_recv.clock_lamport = msg.clock_lamport;
            printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", msg.origin, msg_recv.clock_lamport, msg_recv.origin);
        } else {
            printf("ERROR : Not a valid operation\n");
        }   
    }

    //CLIENTE P1 O P3
    if( i == 0 ) {
        strcpy(p1,msg_recv.origin);
    } else {
        strcpy(p3, msg_recv.origin);
    }
    i++;

    return 0;
}

//INICIA EL THREAD DE UN CLIENTE
void init_thread() {
    pthread_create(&thread, NULL, msg_shutdown, NULL);
}

//THREAD CLIENTE recibe SHUTDOWN_NOW, si no, error.
void *msg_shutdown() {
    while(1) {
        usleep(10000);
        struct message shutdown;
        if ((recv(sockfd, &shutdown, sizeof(shutdown), 0)) != -1) {
            if(shutdown.action == SHUTDOWN_NOW) {
                msg.clock_lamport = act_lamport(shutdown);
                shutdown.clock_lamport = msg.clock_lamport;
                printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", msg.origin, shutdown.clock_lamport, shutdown.origin);
                break;
            } else {
                error("ERROR : Not a valid operation");
            }
        }
        else {
            error("Receive data from client failed...");
        }

    }
}

int sending_shutdown(char name[2]) {
    struct message ack;
    strcpy(ack.origin, msg.origin); 
    int index;
    msg.clock_lamport++;
    ack.clock_lamport = msg.clock_lamport;
    ack.action = SHUTDOWN_NOW; 

    printf("%s, %d, SEND, SHUTDOWN_NOW (%s)\n",ack.origin, ack.clock_lamport, name);

    if (ack.clock_lamport == 4 && strcmp(name, p1) == 0) {
        index = 0;
    }
    else if (ack.clock_lamport == 8 && strcmp(name, p3) == 0) {
        index = 1;
    }
    else {
        error("Clock does not correspond to expected");
    }

    if (send(connfd[index], &ack, sizeof(ack), 0) == -1) {
        printf("ERROR sending the message\n");
    }

    recv_ack(ack, name);

    return 0;
}

int recv_ack(struct message ack, char name[2]) {
    int index;

    if (strcmp(name, p1) == 0) {
        index = 0;
    }
    else if (strcmp(name, p3) == 0) {
        index = 1;
    }
    
    // Esperamos al ack del cliente
    if (recv(connfd[index], &ack, sizeof(ack),0) == -1) {
        printf("Receive data from client failed...\n");
    } else {
        //Esperamos el shotdown ack
        if(ack.action == SHUTDOWN_ACK) {
            msg.clock_lamport = act_lamport(ack); 
            ack.clock_lamport = msg.clock_lamport;
            printf("%s, %d, RECV (%s), SHUTDOWN_ACK\n", msg.origin, ack.clock_lamport, ack.origin);
        }else {
            error("RECV_ACK ERROR : Not a valid operation");
        }
    }

    return 0;
}


int close_server() {
    if(close(sockfd) == -1) {
        error("Close failed");
    }
  return 0;
}

int close_client(){
    if(pthread_join(thread, NULL) == -1) {
      error("join failed");
    }
    if(close(connfd[0]) == -1) {
        error("Close failed");
    }
    if(close(connfd[1]) == -1) {
        error("Close failed");
    }
    if(close(sockfd) == -1) {
        error("Close failed");
    }

    return 0;
}

/*




*/