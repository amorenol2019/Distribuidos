/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 4. Proxy
*/

#include "proxy.h"

#define BACKLOG 2000
//OTHERS
int sockfd = 0;
struct sockaddr_in sock;
struct sockaddr_in sock_serv;
/*////////////////////////////---------------------AUXILIAR FUNCTIONS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

/*////////////////////////////---------------------BROKER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_server (unsigned int port) {
    srand(time(NULL));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }

    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
}

void communicate_server(unsigned int port) {
    const int enable = 1;

    set_server(port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }

    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1) {
        error("Socket bind failed...");
    } 

    if((listen(sockfd, BACKLOG)) == -1) {
        close_server();
        error("Listen failed...");
    }
}

void recv_client() {
    /*Server is waiting for clients*/
    while(1) {
        pthread_t c_server;
        pthread_create(&c_server, NULL, &communicate_client, NULL);
    }
}

void *communicate_client(void *arg) {
    int connfd_;
    struct timeval wait_time_init;

    connfd_ = accept(sockfd,(struct sockaddr *)NULL, NULL);
    if (connfd_ < 0) {
        close(sockfd);
        close(connfd_);
        error("Failed server accept...");
    } else {
        if (gettimeofday(&wait_time_init, 0) == -1) {
            error("Error getting stamp of time");
        }
        printf("[%ld.%ld] Nuevo cliente ($ID) Publicador/Suscriptor conectado : $TOPIC\n", wait_time_init.tv_sec, wait_time_init.tv_usec);
    }

    return 0;
}

/*
Cuando se desconectan
Resumen:
$TOPIC1: M Suscriptores - N Publicadores
$TOPIC2: M Suscriptores - N Publicadores

● Cuando recibe un mensaje de un publicador.
[SECONDS.NANOSECONDS] Recibido mensaje para publicar en topic: $TOPIC -
mensaje: $data - Generó: $time_generated_data
● Cuando envía un mensaje a los suscriptores
[SECONDS.NANOSECONDS] Enviando mensaje en topic $TOPIC a $N suscriptores.
*/

int close_server() {
    if(close(sockfd) == -1) {
        error("Close failed");
    }

    return 0;
}


void ctrlHandlerServer(int num) {
    close_server();
    printf("\n");
    exit(EXIT_SUCCESS);
}

/*/////////////////////////////---------------------PUBLISHER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/


/*/////////////////////////////---------------------SUBSCRIBER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
