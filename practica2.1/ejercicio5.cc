#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUF_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUF_SIZE];
    if (argc < 3) {
        fprintf(stderr,"Uso: %s direccion puerto\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR al abrir socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no existe el host\n");
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR al conectarse");

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        fgets(buffer, BUF_SIZE-1, stdin);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
            error("ERROR al escribir en socket");
        if (buffer[0] == 'Q' && strlen(buffer) <= 2)
            break;
        memset(buffer, 0, BUF_SIZE);
        n = read(sockfd, buffer, BUF_SIZE-1);
        if (n < 0) 
            error("ERROR al leer del socket");
        printf("%s", buffer);
    }

    close(sockfd);
    return 0;
}
