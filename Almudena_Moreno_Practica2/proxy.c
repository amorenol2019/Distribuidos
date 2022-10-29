/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 2. Proxy
*/

#include "proxy.h"

int sockfd = 0, id = 0, counter = 0, p1 = -1, p3 = -1;
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

void set_ip_port (char* ip, unsigned int port) {
    const int enable = 1;

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
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close_server();
        error("setsockopt(SO_REUSEADDR) failed");
    }
}

int get_clock_lamport() {
    return msg.clock_lamport;
}

unsigned int update_lamport(struct message clock) {
    if (clock.clock_lamport < msg.clock_lamport) {
        return msg.clock_lamport + 1;
    } else {
        return clock.clock_lamport + 1;
    }
}

//Choose client p1 or p3
int p1_or_p3(char name[2]) {
    if (strcmp(name, "P1") == 0) {
        return p1;
    } else if (strcmp(name, "P3") == 0) {
        return p3;
    } else {
        error(("Not valid client"));
    }

    return -1;
}

/*////////////////////////////---------------------SERVER---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
//Start connection server
int connect_server() {
    if((bind(sockfd, (struct sockaddr*)&sock_serv, sizeof(sock_serv))) == -1) {
        error("Socket bind failed...");
    } else {
        printf("Socket successfully binded...\n");
    }
    if((listen(sockfd, 2)) == -1) {
        close_server();
        error("Listen failed...");
    } else {
        printf("Server listening...\n");
    }

    return 0;
}

//Server accepts client
void recv_client() {
    pthread_create(&thread[2], NULL, &msg_ready, NULL);
    if(pthread_join(thread[2], NULL) == -1) {
        close_server();
        error("join failed");
    }
}

void *msg_ready() {
    struct message msg_recv;

    while(counter < 2) {
        connfd[id] = accept(sockfd,(struct sockaddr*)NULL, NULL);
        if (connfd[id] == -1){
            close_server();
            error("Failed server accept...");
        }
        if ((recv(connfd[id], &msg_recv, sizeof(msg_recv),0)) == -1) {
            printf("Receive data from client failed...\n");
        }else {
            // READY_TO_SHUTDOWN
            if(msg_recv.action == READY_TO_SHUTDOWN) {
                // Actualizamos reloj
                msg.clock_lamport = update_lamport(msg_recv); 
                printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", msg.origin, msg.clock_lamport, msg_recv.origin);
            } else {
                close_server();
                error("ERROR : Not a valid operation\n");
            }   
        }
        
        //CLIENTE P1 O P3
        if(strcmp("P1", msg_recv.origin) == 0) {
            p1 = id;
        } else if (strcmp("P3", msg_recv.origin) == 0) {
            p3 = id;
        } 
        id++;
        counter++;
    }
    if (p1 == -1 || p3 == -1) {
        close_server();
        error("Needed a client called P1, and another one called P3");
    }

    return 0;
}

int notify_shutdown_now(char name[2]) {
    struct message ack;
    int index;
    
    strcpy(ack.origin, msg.origin); 
    msg.clock_lamport++;
    ack.clock_lamport = msg.clock_lamport;
    ack.action = SHUTDOWN_NOW; 

    if (msg.clock_lamport == 4 && strcmp(name, "P1") == 0) {
        index = p1;
    } else if (msg.clock_lamport == 8 && strcmp(name, "P3") == 0) {
        index = p3;
    } else {
        close_server();
        error("Clock does not correspond to expected");
    }

    if (send(connfd[index], &ack, sizeof(ack), 0) == -1) {
        close_server();
        error("ERROR sending the message\n");
    }
    printf("%s, %d, SEND, SHUTDOWN_NOW (%s)\n",ack.origin, msg.clock_lamport, name);

    return 0;
}

//Receive ack (index - num of client)
void recv_ack(char name[2]) {
    int client = p1_or_p3(name);
    pthread_create(&thread[2], NULL, msg_ack, &client);
    if(pthread_join(thread[2], NULL) == -1) {
        close_server();
        error("join failed");
    }
}

void *msg_ack(void *idc) {
    int index = *(int *) idc;
    
    while ( msg.clock_lamport != 7 && msg.clock_lamport != 11) {
        struct message ack;

        if (recv(connfd[index], &ack, sizeof(ack), 0) == -1) {
            printf("Receive data from client failed...\n");
        } else {
            if(ack.action == SHUTDOWN_ACK) {
                msg.clock_lamport = update_lamport(ack);
                if ((index == p1 && msg.clock_lamport == 7) || (index == p3 && msg.clock_lamport == 11)) {
                    printf("%s, %d, RECV (%s), SHUTDOWN_ACK\n", msg.origin, msg.clock_lamport, ack.origin);
                } else {
                    close_server();
                    error("Clock does not correspond to expected.");
                }
            } else {
                close_server();
                error("RECV_ACK ERROR : Not a valid operation");
            }
        }
    }

    return 0;
}

int close_server() {
    if(close(sockfd) == -1) {
        error("Close failed");
    }
    for (int i = 0; i < counter; i++) {
        if(close(connfd[i]) == -1) {
            error("Close failed");
        }
    }
    
    return 0;
}

void ctrlHandlerServer(int num) {
    close_server();
    exit(EXIT_SUCCESS);
}

/*/////////////////////////////---------------------CLIENTS---------------------\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

//Connect clients with server
int connect_client() {
    /*Create a socket and test*/
    if((connect(sockfd, (struct sockaddr*)&sock, sizeof(sock))) == -1) {
        error("Connection with the server failed...");
    } else {
        printf("Connected to the server...\n");
    }

    return 0;
}

void notify_ready_shutdown() {
    struct message msg2;

    strcpy(msg2.origin, msg.origin);
    msg.clock_lamport++;
    msg2.clock_lamport = msg.clock_lamport;
    msg2.action = READY_TO_SHUTDOWN;
    if (send(sockfd, &msg2, sizeof(msg2), 0) == -1) {
        printf("Send to server failed...\n");
    }
    printf("%s, %d, SEND, READY_TO_SHUTDOWN\n", msg2.origin, msg2.clock_lamport);
}

//Receive shutdown_now from client
void recv_shutdown_now(char id_client[2]) {
    int client = p1_or_p3(id_client);

    pthread_create(&thread[client], NULL, msg_shutdown, &client);
}

void *msg_shutdown(void *idc) {
    struct message shutdown;
    int client = *(int *) idc;
    
    while (msg.clock_lamport != 4 && msg.clock_lamport != 8) {
        if ((recv(sockfd, &shutdown, sizeof(shutdown), 0)) != -1) {
            if(shutdown.action == SHUTDOWN_NOW) {
                msg.clock_lamport = update_lamport(shutdown);
                if ((client == p1 && msg.clock_lamport == 5) || (client == p3 && msg.clock_lamport == 9)) {
                    printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", msg.origin, msg.clock_lamport, shutdown.origin);
                } else {
                    close_client();
                    printf("CLIENT %d %d\n", client, msg.clock_lamport);
                    error("Clock does not correspond to expected");
                }
            } else {
                close_client();
                error("ERROR : Not a valid operation");
            }
        } else {
            close_client();
            error("Receive data from client failed...");
        }
    }

    return 0;
}

void notify_shutdown_ack() {
    struct message msg2;

    strcpy(msg2.origin, msg.origin);
    msg.clock_lamport++;
    msg2.clock_lamport = msg.clock_lamport;
    msg2.action = SHUTDOWN_ACK;
    if (send(sockfd, &msg2, sizeof(msg2), 0) == -1) {
        printf("Send to server failed...\n");
    }
    printf("%s, %d, SEND, SHUTDOWN_ACK\n", msg2.origin, msg2.clock_lamport);
}

int close_client(){
    if(close(sockfd) == -1) {
        error("Close failed");
    }

    return 0;
}

void ctrlHandler(int num) {
    close_client();
    exit(EXIT_SUCCESS);
}
