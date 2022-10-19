/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 3. Servidor Multi Hilo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX 1024
#define NCLIENTS 100

int tcp_socket = 0;

void error(char *msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

void ctrlHandler(int num) {
    close(tcp_socket);
    exit(EXIT_SUCCESS);
}

void *thread_client(void *arg) {
    char buff[MAX];
    bzero(buff, MAX);
    fd_set readmask;
    struct timeval timeout;
    int connfd_ = *(int *)arg, r = 0;
    //*Select: monitors file descriptos until readyy to I/O operations*/
    FD_ZERO(&readmask); // Reset mask
    FD_SET(connfd_, &readmask); // Assign new descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entry
    timeout.tv_sec = 2; timeout.tv_usec = 500000; // Timeout de 2.5 seg.
    
    if (select(connfd_ + 1, &readmask, NULL, NULL, &timeout) == -1) {
        close(connfd_);
        close(tcp_socket);
        error("Error in select...");
    }

    if (FD_ISSET(connfd_, &readmask)) {
        /*Receive data from a socket*/
        r = recv(connfd_, (void*) buff, sizeof(buff), 0);
        if (r == -1) {
            close(connfd_);
            close(tcp_socket);
            error("Receive data from client failed...");
        }
        printf("+++ ");
        if (fputs(buff, stdout) == EOF) {
            close(connfd_);
            close(tcp_socket);
            error("[ERROR]: fputs() failed...");
        }
    }

    /*Get message to client*/
    char *message = "Hello client!\n";
    if (send(connfd_, message, strlen(message), 0) == -1) {
        close(connfd_);
        close(tcp_socket);
        error("ERROR sending the message");
    }
    //Test multi threads: sleep(4);
    if (close(connfd_) == -1) {
        close(tcp_socket);
        error("Error closing socket");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    if(argc != 2) {
        error("Incorrect program call.\n Usage: ./server PORT");
    }
    int port = atoi(argv[1]);

    const int enable = 1;
    struct sockaddr_in sock_serv;
    bzero(&sock_serv, sizeof(sock_serv)); //Erase data
    struct sockaddr_in sock_cli;
    bzero(&sock_cli, sizeof(sock_cli));
    socklen_t len;

    signal(SIGINT, ctrlHandler);   //Close with CTRL + C

    /*Create a socket and test*/
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }
    
    /*Create IP direction and port*/
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);   //Any interface
    sock_serv.sin_port = htons(port); 

    /*Close socket without time wait*/
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close(tcp_socket);
        error("setsockopt(SO_REUSEADDR) failed");
    }

    /*Assign specific direction to the socket and test*/
    if (bind(tcp_socket, (struct sockaddr *) &sock_serv, sizeof(sock_serv)) == -1) {
        close(tcp_socket);
        error("Socket bind failed...");
    } else {
        printf("Socket successfully binded...\n");
    }
    
    /*Server listening*/
    if (listen(tcp_socket, 200) == -1) {
        close(tcp_socket);
        error("Listening failed...");
    } else {
        printf("Server listening...\n");
    }

    while(1) {
        /*Server is waiting for clients*/
        pthread_t thread[NCLIENTS];
        int connfd[NCLIENTS];

        for (int i = 0; i < NCLIENTS; i++) {
            len = sizeof(sock_cli);
            connfd[i] = accept(tcp_socket,(struct sockaddr *)&sock_cli, &len);
            if (connfd[i] < 0) {
                close(connfd[i]);
                close(tcp_socket);
                error("Failed server accept...");
            }
            if (pthread_create(&thread[i], NULL, &thread_client, &connfd[i]) == -1) {
                close(connfd[i]);
                close(tcp_socket);
                error("Error creating thread");
            }
        }

        //Closing threads
        for (int i = 0; i < NCLIENTS; i++) {
            if (pthread_join(thread[i], NULL) == -1) {
                close(connfd[i]);
                close(tcp_socket);
                error("Error closing thread");
            }
        }
    }
}