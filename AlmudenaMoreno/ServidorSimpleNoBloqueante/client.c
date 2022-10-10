/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 1. Cliente Simple
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 256
#define PORT 8080
#define FAIL 1

int client_socket = 0;

void error(char *msg) {
    printf("%s", msg);
    exit(FAIL);
}

void ctrlHandler(int num) {
    if (close(client_socket) == -1) {
        error("Client not correctly closed...\n");
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int r = 0;
    signal(SIGINT, ctrlHandler);   //Cierra con control + c

    char buff[MAX];
    explicit_bzero(buff, MAX);  //Erase data if necessary 

    /*Create a socket and test*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        error("Socket creation failed...\n");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Server is waiting for clients*/
    /*Create IP direction and port*/
    struct sockaddr_in servaddr;
    explicit_bzero(&servaddr, sizeof(servaddr));  //Erase data if necessary 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("212.128.254.3");   //Any interface
    servaddr.sin_port = htons(PORT); 
    socklen_t len = sizeof(servaddr);
    if (connect(client_socket, (struct sockaddr *)&servaddr, len) == -1) {
        error("Server connection failed...\n");
    } else { 
        printf("connected to the server...\n");
    }

    while(1) {
        /*Send message to server*/
        printf("> ");
        char message[MAX];
        fgets(message, MAX, stdin);
        send(client_socket, message, strlen(message), 0);

        /*Select: monitors file descriptos until readyy to I/O operations*/
        fd_set readmask;
        struct timeval timeout;
        FD_ZERO(&readmask); // Reset la mascara
        FD_SET(client_socket, &readmask); // Asignamos el nuevo descriptor
        FD_SET(STDIN_FILENO, &readmask); // Entrada
        timeout.tv_sec = 0; timeout.tv_usec = 500000; // Timeout de 0.5 seg.
        if (select(client_socket + 1, &readmask, NULL, NULL, &timeout) == -1) {
            exit(FAIL);
        }
        /*Data to read from descriptor*/
        if (FD_ISSET(client_socket, &readmask)) {
            /*Receive data from a socket*/
            r = recv(client_socket, (void*) buff, sizeof(buff), MSG_DONTWAIT);
            if (r > 0) {
                printf("+++ ");
                if (fputs(buff, stdout) == EOF) {
                    fprintf(stderr, "[ERROR]: fputs() failed...");
                }
            } else if (r == -1) {
                error("Receive data from server failed...\n");
            }
        }
    }

    return 0;
}