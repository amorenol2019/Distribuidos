/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 1. Servidor Simple
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX 256
#define PORT 8080
#define NCLIENTS 1
#define FAIL 1

int tcp_socket = 0;

void error(char *msg) {
    printf("%s", msg);
    exit(FAIL);
}

void ctrlHandler(int num) {
    if (close(tcp_socket) == -1) {
        error("\nServer not correctly closed...\n");
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    const int enable = 1;
    int list = 0, connfd = 0, res = 0, r = 0;

    signal(SIGINT, ctrlHandler);   //Cierra con control + c

    char buff[MAX];
    bzero(buff, MAX); //Erase data if necessary 

    fd_set readmask;
    struct timeval timeout;

    /*Create a socket and test*/
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        error("Socket creation failed...\n");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Create IP direction and port*/
    struct sockaddr_in servaddr;
    explicit_bzero(&servaddr, sizeof(servaddr));  //Erase data if necessary 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //Any interface
    servaddr.sin_port = htons(PORT); 

    /*Reuse ports*/
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        error("setsockopt(SO_REUSEADDR) failed");
    }
    /*Assign specific direction to the socket and test*/
    res = bind(tcp_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (res == -1) {
        error("Socket bind failed...\n");
    } else {
        printf("Socket successfully binded...\n");
    }
    
    /*Server listening*/
    list = listen(tcp_socket, NCLIENTS);
    if (list == -1) {
        error("Listening failed...\n");
    } else {
        printf("Server listening...\n");
    }

    /*First conection from stack, create a new socket for client*/
    struct sockaddr_in sock_cli;
    socklen_t len = sizeof(sock_cli);

    /*Server is waiting for clients*/
    connfd = accept(tcp_socket,(struct sockaddr *)&sock_cli, &len);
    if (connfd < 0) {
        error("Failed server accept...\n");
    }

    while(1) {
        /*Select: monitors file descriptos until readyy to I/O operations*/
        FD_ZERO(&readmask); // Reset la mascara
        FD_SET(connfd, &readmask); // Asignamos el nuevo descriptor
        FD_SET(STDIN_FILENO, &readmask); // Entrada
        //NULL Timeout - undefined waiting time
        timeout.tv_sec = 0; timeout.tv_usec = 0; // Timeout de 0.5 seg.
        if (select(connfd + 1, &readmask, NULL, NULL, &timeout) == -1) {
            exit(FAIL);
        }
        /*Data to read from descriptor*/
        if (FD_ISSET(connfd, &readmask)) {
            /*Receive data from a socket*/
            r = recv(connfd, (void*) buff, sizeof(buff), 0);
            if (r > 0) {
                printf("+++ ");
                if (fputs(buff, stdout) == EOF) {
                    error("[ERROR]: fputs() failed...\n");
                }
            } else if (r == -1) {
                error("Receive data from client failed...\n");
            }
        }
        
        /*PARA QUE HAY QUE UTILIZAR SCANF*/
        /*Get message to client*/
        printf("> ");
        char message[MAX];
        fgets(message, MAX, stdin);
        send(connfd, message, strlen(message), 0);
    }

    return 0;
}