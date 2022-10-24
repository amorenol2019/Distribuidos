/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. Proxy
*/

#include <proxy.h>

//Variables para el cliente y el servidor
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

    if (send(sockfd, &msg2, sizeof(ready), 0) < 0)
    {
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

    if (send(sockfd, &msg2, sizeof(ready), 0) < 0)
    {
        printf("Send to server failed...\n");
    }
}

// Actualizamos el reloj de lamport
unsigned int lamport_act(struct message lamport) {
    unsigned int counter;
    if (lamport.clock_lamport < msg.clock_lamport) {
        counter = msg.clock_lamport;
    }else {
        counter = lamport.clock_lamport;
    }
    msg.clock_lamport = counter;
    msg.clock_lamport++;

    return msg.clock_lamport;
}

int connect_client() {
    /*Create a socket and test*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }

    if((connect(sockfd, (struct sockaddr*)&sock_cli, sizeof(sock_cli))) < 0){
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
    if((bind(sockfd,(struct sockaddr*)&sock_serv,sizeof(sock_serv))) != 0){
        error("Socket bind failed...");
    } else {
        printf("Socket successfully binded...\n");
    }
    if((listen(sockfd, 2)) != 0){
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
    if(pthread_join(thread, NULL) != 0) {
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
    pthread_create(&thread, NULL, wait_message, NULL);
}

void *wait_message() {
    while(1) {
        struct message shutdown;
        usleep(5000);
        if ((recv(sockfd, &shutdown, sizeof(shutdown),MSG_DONTWAIT)) > 0) {
            if(shutdown.action == SHUTDOWN_NOW) {
                shutdown.clock_lamport = lamport_increase(shutdown);
                printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", message.origin, shutdown.clock_lamport, shutdown.origin);
                break;
            } else {
                printf("Wrong operation\n");
            }
        }

    }

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