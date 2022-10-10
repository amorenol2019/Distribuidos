/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 1. Servidor Simple
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAX 256
#define PORT 8080
#define NCLIENTS 1

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int tcp_socket = 0, list = 0, connfd = 0, res = 0, r = 0;

    char buff[MAX];
    bzero(buff, MAX); //Erase data if necessary 

    /*Create IP direction and port*/
    struct sockaddr_in servaddr;
    //int true_bool = 1;
    //Cerrar socket sin time wait?
    //setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &true_bool, sizeof(int));
    explicit_bzero(&servaddr, sizeof(servaddr));  //Erase data if necessary 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //Any interface
    servaddr.sin_port = htons(PORT); 

    /*Create a socket and test*/
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    } else {
        printf("Socket successfully created...\n");
    }
    //udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    /*Assign specific direction to the socket and test*/
    res = bind(tcp_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (res == -1) {
        printf("Socket bind failed...\n");
        exit(1);
    } else {
        printf("Socket successfully binded...\n");
    }
    
    /*Server listening*/
    list = listen(tcp_socket, NCLIENTS);
    if (list == -1) {
        printf("Listening failed...\n");
        exit(1);
    } else {
        printf("Server listening...\n");
    }

    /*First conection from stack, create a new socket for client*/
    struct sockaddr_in sock_cli;
    socklen_t len = sizeof(sock_cli);

    /*Server is waiting for clients*/
    connfd = accept(tcp_socket,(struct sockaddr *)&sock_cli, &len);
    if (connfd < 0) {
        printf("Failed server accept...\n");
        exit(1);
    }

    /*Receive data from a socket*/
    r = recv(connfd, (void*) buff, sizeof(buff), 0);
    if (r > 0) {
        printf("+++ ");
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "[ERROR]: fputs() failed...");
        }
    } else if (r == -1) {
        printf("Receive data from client failed...\n");
        exit(1);
    }
    /*PARA QUE HAY QUE UTILIZAR SCANF*/
    /*Get message to client*/
    printf("> ");
    char message[MAX];
    fgets(message, MAX, stdin);
    send(connfd, message, strlen(message), 0);

    /*Socket closed*/
    if (close(tcp_socket) < 0 || close(connfd) < 0) {
        printf("Client not correctly closed...\n");
        exit(1);
    }

    return 0;
}