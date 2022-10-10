/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 1. Cliente Simple
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX 256
#define PORT 8080

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int tcp_socket = 0, r = 0;

    char buff[MAX];
    explicit_bzero(buff, MAX);  //Erase data if necessary 

    /*Create a socket and test*/
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    } else {
        printf("Socket successfully created...\n");
    }

    /*Server is waiting for clients*/
    /*Create IP direction and port*/
    struct sockaddr_in servaddr;
    explicit_bzero(&servaddr, sizeof(servaddr));  //Erase data if necessary 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //Any interface
    servaddr.sin_port = htons(PORT); 
    socklen_t len = sizeof(servaddr);
    if (connect(tcp_socket, (struct sockaddr *)&servaddr, len) == -1) {
        printf("Server connection failed...\n");
        exit(1);
    } else { 
        printf("connected to the server...\n");
    }

    /*Send message to server*/
    printf("> ");
    char message[MAX];
    fgets(message, MAX, stdin);
    send(tcp_socket, message, strlen(message), 0);

    /*Select: monitors file descriptos until readyy to I/O operations*/
    fd_set readmask;
    struct timeval timeout;
    FD_ZERO(&readmask); // Reset la mascara
    FD_SET(tcp_socket, &readmask); // Asignamos el nuevo descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entrada
    timeout.tv_sec = 0; timeout.tv_usec = 500000; // Timeout de 0.5 seg.
    if (select(tcp_socket + 1, &readmask, NULL, NULL, &timeout) == -1) {
        exit(1);
    }
    /*Data to read from descriptor*/
    if (FD_ISSET(tcp_socket, &readmask)) {
        /*Receive data from a socket*/
        r = recv(tcp_socket, (void*) buff, sizeof(buff), MSG_DONTWAIT);
        if (r > 0) {
            printf("+++ ");
            if (fputs(buff, stdout) == EOF) {
                fprintf(stderr, "[ERROR]: fputs() failed...");
            }
        } else if (r == -1) {
            printf("Receive data from server failed...\n");
            exit(1);
        }
    }

    /*Socket closed*/
    if (close(tcp_socket) < 0) {
        printf("Client not correctly closed...\n");
        exit(1);
    }

    return 0;
}