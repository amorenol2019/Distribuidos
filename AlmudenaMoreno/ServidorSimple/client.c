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
    if (close(client_socket) < 0) {
        error("Client not correctly closed...\n");
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int r = 0, run = 1;
    signal(SIGINT, ctrlHandler);   //Cierra con control + c

    char buff[MAX];
    explicit_bzero(buff, MAX);  //Erase data if necessary 

    /*Create IP direction and port*/
    struct sockaddr_in servaddr;
    explicit_bzero(&servaddr, sizeof(servaddr));  //Erase data if necessary 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("212.128.254.4");   //Any interface
    servaddr.sin_port = htons(PORT); 

    /*Create a socket and test*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        error("Socket creation failed...\n");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Server is waiting for clients*/
    socklen_t len = sizeof(servaddr);
    if (connect(client_socket, (struct sockaddr *)&servaddr, len) == -1) {
        error("Server connection failed...\n");
    } else { 
        printf("connected to the server...\n");
    }

    while (1) {
        /*Send message to server*/
        printf("> ");
        char message[MAX];
        fgets(message, MAX, stdin);
        send(client_socket, message, strlen(message), 0);

        /*Receive data from a socket*/
        r = recv(client_socket, (void*) buff, sizeof(buff), 0);
        if (r > 0) {
            printf("+++ ");
            if (fputs(buff, stdout) == EOF) {
                error("[ERROR]: fputs() failed...\n");
            }
        }

        /*Socket closed
        
        */
    }

    return 0;
}