/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 3. Proxy


Semáforos, variables condición, mutex

*/

#include "proxy.h"

#define backlog 2000
#define MAX_CLIENTS 250
#define L_SIZE 250

void error(char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

int sockfd = 0, counter = 0, roll;
struct sockaddr_in sock;
struct sockaddr_in sock_serv;
int connfd; //[MAX_CLIENTS];
char fline[L_SIZE];

FILE* file;

struct request msg_recv;
struct response msg_serv;
struct request msg_client;
struct response msg_resp;


/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_client (unsigned int port) {
    const int enable = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }
    
    bzero(&sock_serv, sizeof(sock_serv));
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_serv.sin_port = htons(port);
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }

    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1) {
        error("Socket bind failed...");
    } 
    if((listen(sockfd, backlog)) == -1) {
        close_server();
        error("Listen failed...");
    }
    if ((file = fopen("server_output.txt", "a+")) == NULL) {
        error("Error opening file");
    }
    if (fgets(fline, L_SIZE, file) == NULL) {
        msg_serv.counter = 0;
    } else {
        msg_serv.counter = atoi(fline);
    }
}

void recv_client() {
    /*Server is waiting for clients*/
    connfd = accept(sockfd,(struct sockaddr *)NULL, NULL);
    if (connfd < 0) {
        close(sockfd);
        close(connfd);
        error("Failed server accept...");
    }
    if ((recv(connfd, &msg_recv, sizeof(msg_recv), 0)) < 0) {
        error("Recv from the client failed...\n");
    }

    printf("[SECONDS.MICRO] ");
    if (msg_recv.action == WRITE) {
        msg_serv.counter++;

        if (fprintf(file, "%d\n", msg_serv.counter) < 0) {
            error("Error writing file");
        }
        printf("[ESCRITOR #%d] modifica contador con valor\n", msg_recv.id);
    } else if (msg_recv.action == READ) {
        printf("[LECTOR #%d] lee contador con valor\n", msg_recv.id);
    } else {
        error("Not action allowed");
    }

    msg_serv.action = msg_recv.action;
    msg_serv.waiting_time = 0;

    if (send(connfd, &msg_serv, sizeof(msg_serv), 0) < 0) {
        printf("Send to server failed...\n");
    }
    printf("SEND response COUNTER %d, WAITING TIME, %ld, ACTION, %d\n", msg_serv.counter, msg_serv.waiting_time, msg_serv.action);
}

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

/*/////////////////////////////---------------------CLIENTS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void set_ip_port (char* ip, unsigned int port) {
    bzero(&sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(ip);
    sock.sin_port = htons(port);
}

void create_socket() {
    const int enable = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Socket creation failed...");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }
}

void read_or_write(char* ip, int port, int threads, char* mode) {
    //pthread_t clients[threads];

    if (strcmp(mode, "reader") == 0) {
        roll = 1;

    } else if (strcmp(mode, "writer") == 0) {
        roll = 0;
    }

    set_ip_port(ip, port);

    int t_id = 0;

    create_socket();
    if((connect(sockfd, (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    }

    msg_client.action = roll;
    msg_client.id = t_id;

    if (send(sockfd, &msg_client, sizeof(msg_client), 0) == -1) {
        error("Send to server failed...\n");
    }
    printf("[CLIENT #%d] Send %d\n", msg_client.id, msg_client.action);


    if ((recv(sockfd, (void *) &msg_resp, sizeof(msg_resp), 0)) == -1) {
        error("Receive message failed...");
    } else {
        if (msg_resp.action == WRITE) {
            printf("Cliente #%d Escritor, contador = %d, tiempo = %ld ns. \n", t_id, msg_resp.counter, msg_resp.waiting_time);
        } else if (msg_resp.action == READ) {
            printf("Cliente #%d Lector, contador = %d, tiempo = %ld ns. \n", t_id, msg_resp.counter, msg_resp.waiting_time);
        } else {
            printf("Response action %d\n", msg_resp.action);
        }
    }
}

int close_client(){
    if(close(sockfd) == -1) {
        error("Close failed");
    }

    return 0;
}

void ctrlHandler(int num) {
    close_client();
    printf("\n");
    exit(EXIT_SUCCESS);
}
