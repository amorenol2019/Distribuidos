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
#define FAIL 1

int tcp_socket = 0;

void error(char *msg) {
    printf("%s", msg);
    close(tcp_socket);
    exit(FAIL);
}

void ctrlHandler(int num) {
    close(tcp_socket);
    exit(0);
}

void *thread_client(void *arg) {
    char buff[MAX];
    bzero(buff, MAX);
    fd_set readmask;
    struct timeval timeout;
    int connfd = *(int *)arg, r = 0;

    //*Select: monitors file descriptos until readyy to I/O operations*/
    FD_ZERO(&readmask); // Reset la mascara
    FD_SET(connfd, &readmask); // Asignamos el nuevo descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entrada
    //NULL Timeout - undefined waiting time
    timeout.tv_sec = 3; timeout.tv_usec = 500000; // Timeout de 3 seg.
    
    if (select(connfd + 1, &readmask, NULL, NULL, &timeout) == -1) {
        printf("ERROR");
        exit(EXIT_FAILURE);
    }

    if (FD_ISSET(connfd, &readmask)) {
        /*Receive data from a socket*/
        r = recv(connfd, (void*) buff, sizeof(buff), 0);
        if (r == -1) {
            error("Receive data from client failed...\n");
        } else if (r > 0) {
            printf("+++ ");
            if (fputs(buff, stdout) == EOF) {
                error("[ERROR]: fputs() failed...\n");
            }
        }
    }
    /*Get message to client*/
    printf("> ");
    char *message = "Hello client!\n";
    printf("%s", message);
    send(connfd, message, strlen(message), 0);
    
    if (close(connfd) == -1) {
        error("Error closing socket");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    //Numero de argumentos incluyendo el nombre del programa
    if(argc != 2) {
        printf("Incorrect program call.\n Usage: ./server PORT");
        exit(1);
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
        error("Socket creation failed...\n");
    } else {
        printf("Socket successfully created...\n");
    }
    
    /*Create IP direction and port*/
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = htonl(INADDR_ANY);   //Any interface
    sock_serv.sin_port = htons(port); 

    /*Close socket without time wait*/
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        error("setsockopt(SO_REUSEADDR) failed");
    }

    /*Assign specific direction to the socket and test*/
    if (bind(tcp_socket, (struct sockaddr *) &sock_serv, sizeof(sock_serv)) == -1) {
        error("Socket bind failed...\n");
    } else {
        printf("Socket successfully binded...\n");
    }
    
    /*Server listening*/
    if (listen(tcp_socket, 200) == -1) {
        error("Listening failed...\n");
    } else {
        printf("Server listening...\n");
    }

    /*First conection from stack, create a new socket for client*/
    len = sizeof(sock_cli);

    while(1) {
        /*Server is waiting for clients*/
        pthread_t thread[NCLIENTS];
        for (int i = 0; i < NCLIENTS; i++) {
            int connfd = accept(tcp_socket,(struct sockaddr *)&sock_cli, &len);
            if (connfd < 0) {
                error("Failed server accept...\n");
            }
            if (pthread_create(&thread[i], NULL, &thread_client, &connfd) == -1) {
                error("Error creating thread\n");
            }
            printf("%d\n", i);
        }

        printf("-------------------------------CREADOS----------------------\n");

        for (int i = 0; i < NCLIENTS; i++) {
            if (pthread_join(thread[i], NULL) == -1) {
                error("Error closing thread");
            }
        }
        printf("-------------------------------CERRADOS----------------------\n");
    }
}