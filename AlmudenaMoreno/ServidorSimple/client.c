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

#define MAX 1024
#define PORT 8080
#define FAIL 1

int client_socket = 0;

void error(char *msg) {
    printf("%s", msg);
    close(client_socket);
    exit(FAIL);
}

void ctrlHandler(int num) {
    close(client_socket);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    int r = 0;
    char buff[MAX];
    bzero(buff, MAX);  //Erase data
    char message[MAX];
    bzero(message, MAX);
    struct sockaddr_in sock_serv;
    bzero(&sock_serv, sizeof(sock_serv));
    socklen_t len;

    signal(SIGINT, ctrlHandler);   //Close with CTRL + C

    /*Create a socket and test*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        error("Socket creation failed...\n");
    } else {
        printf("Socket successfully created...\n");
    }

    /*Create IP direction and port*/
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_addr.s_addr = inet_addr("127.0.0.1");//"212.128.254.23");   //Any interface
    sock_serv.sin_port = htons(PORT); 

    /*Server is waiting for clients*/
    len = sizeof(sock_serv);
    if (connect(client_socket, (struct sockaddr *)&sock_serv, len) == -1) {
        error("Server connection failed...\n");
    } else { 
        printf("connected to the server...\n");
    }

    while (1) {
        /*Send message to server*/
        printf("> ");
        fgets(message, MAX, stdin);
        send(client_socket, message, strlen(message), 0);

        /*Receive data from a socket*/
        r = recv(client_socket, (void*) buff, sizeof(buff), 0);
        if (r == -1) {
            error("Receive data from client failed...\n");
        } else if (r > 0) {
            printf("+++ ");
            if (fputs(buff, stdout) == EOF) {
                error("[ERROR]: fputs() failed...\n");
            }
        }
    }

    return 0;
}
