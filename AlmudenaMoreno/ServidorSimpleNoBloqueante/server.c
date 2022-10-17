/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 2. Servidor Simple No Bloqueante
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX 1024
#define PORT 8080
#define NCLIENTS 1
#define FAIL 1

int tcp_socket = 0;

void error(char *msg) {
    printf("%s", msg);
    close(tcp_socket);
    exit(FAIL);
}

void ctrlHandler(int num) {
    close(tcp_socket);
    exit(FAIL);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    const int enable = 1;
    int connfd = 0, r = 0;
    struct sockaddr_in sock_serv;
    bzero(&sock_serv, sizeof(sock_serv)); //Erase data
    struct sockaddr_in sock_cli;
    bzero(&sock_cli, sizeof(sock_cli));
    socklen_t len;
    char buff[MAX];
    bzero(buff, MAX);
    char message[MAX];
    bzero(message, MAX);
    fd_set readmask;
    struct timeval timeout;

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
    sock_serv.sin_port = htons(PORT); 

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
    if (listen(tcp_socket, NCLIENTS) == -1) {
        error("Listening failed...\n");
    } else {
        printf("Server listening...\n");
    }

    /*First conection from stack, create a new socket for client*/
    len = sizeof(sock_cli);

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
            if (r == -1) {
                error("Receive data from client failed...\n");
            } else if (r > 0) {
                printf("+++ ");
                if (fputs(buff, stdout) == EOF) {
                    error("[ERROR]: fputs() failed...\n");
                }
            }
        }
        
        /*PARA QUE HAY QUE UTILIZAR SCANF*/
        /*Get message to client*/
        printf("> ");
        fgets(message, MAX, stdin);
        send(connfd, message, strlen(message), 0);
    }

    return 0;
}