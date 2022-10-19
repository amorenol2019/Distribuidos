/*
Almudena Moreno Lopez
Sistemas Distribuidos y Concurrentes
Práctica 1. Ejercicio 2. Cliente Simple No Bloqueante
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 1024
#define PORT 8080

int client_socket = 0;

void error(char *msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

void ctrlHandler(int num) {
    close(client_socket);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int r = 0;
    struct sockaddr_in sock_serv;
    bzero(&sock_serv, sizeof(sock_serv));
    socklen_t len;
    char buff[MAX];
    bzero(buff, MAX);
    char message[MAX];
    bzero(message, MAX);
    fd_set readmask;
    struct timeval timeout;

    signal(SIGINT, ctrlHandler);   //Close with CTRL + C

    /*Create a socket and test*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        close(client_socket);
        error("Socket creation failed...");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Create IP direction and port*/
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = inet_addr("127.0.0.1"); //Any interface
    sock_serv.sin_port = htons(PORT); 

    /*Server is waiting for clients*/
    len = sizeof(sock_serv);
    if (connect(client_socket, (struct sockaddr *)&sock_serv, len) == -1) {
        close(client_socket);
        error("Server connection failed...");
    } else { 
        printf("connected to the server...\n");
    }

    while(1) {
        /*Send message to server*/
        printf("> ");
        fgets(message, MAX, stdin);
        send(client_socket, message, strlen(message), 0);

        //*Select: monitors file descriptos until readyy to I/O operations*/
        FD_ZERO(&readmask); // Reset mask
        FD_SET(client_socket, &readmask); // Assign new descriptor
        FD_SET(STDIN_FILENO, &readmask); // Entry
        timeout.tv_sec = 0; timeout.tv_usec = 500000; // Timeout de 0.5 seg.
        if (select(client_socket + 1, &readmask, NULL, NULL, &timeout) == -1) {
            error("Select failed...");
        }
        /*Data to read from descriptor*/
        if (FD_ISSET(client_socket, &readmask)) {
            /*Receive data from a socket*/
            r = recv(client_socket, (void*) buff, sizeof(buff), 0);
            if (r > 0) {
                printf("+++ ");
                if (fputs(buff, stdout) == EOF) {
                    close(client_socket);
                    fprintf(stderr, "[ERROR]: fputs() failed...");
                }
            } else if (r == -1) {
                close(client_socket);
                error("Receive data from server failed...");
            }
        }
    }

    return 0;
}