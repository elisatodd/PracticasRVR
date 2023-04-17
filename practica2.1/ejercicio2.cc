#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include <time.h>

#define BUF_SIZE 500

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s direction port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0){
            printf("Conexión realizada\n");
            break;                  /* Success */
        }

        close(sfd);
    }

    freeaddrinfo(result);

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    int bytes;
    char buffer[BUF_SIZE];
    char bufferRespuesta[BUF_SIZE];
    struct sockaddr cliente;
    socklen_t size = sizeof(cliente);

    char host[BUF_SIZE];
    char serv[BUF_SIZE];

    while(1)
    {
        bytes = recvfrom(sfd, buffer, BUF_SIZE, 0, &cliente, &size);
        
        if (bytes == -1){
            perror("recvfrom\n");
        }else if (bytes == 0){
            break;
        }

        buffer[bytes] = '\0';
        char c = ' ';
        int i = 0;

        printf("%d bytes de %s:%s\n", bytes, argv[1], argv[2]);

        if (getnameinfo(&cliente, size, host, BUF_SIZE, serv, BUF_SIZE, NI_NUMERICHOST | NI_NUMERICSERV) == -1){

            perror("getnameinfo\n");
        }

        if (buffer[0] == 't')
        {
            time_t t = time(NULL);
            struct tm* tm_info = localtime(&t);
            int respSize = strftime(bufferRespuesta, BUF_SIZE, "%r", tm_info); 
            bufferRespuesta[respSize] = '\n';
            sendto(sfd, bufferRespuesta, respSize+1, 0, (struct sockaddr *)&cliente, size);
        }else if (buffer[0] == 'd')
        {
            time_t t = time(NULL);
            struct tm* tm_info = localtime(&t);
            int respSize = strftime(bufferRespuesta, BUF_SIZE, "%F", tm_info); 
            bufferRespuesta[respSize] = '\n';
            sendto(sfd, bufferRespuesta, respSize+1, 0, (struct sockaddr *)&cliente, size);
        }else if (buffer[0] == 'q')
        {
            printf("Quit\n");
            break;
        }else
        {
            printf("Uso: t para ver la hora | d para ver la fecha | q para terminar\n");
        }
    }

    close(sfd);
    // if (getnameinfo(rp->ai_addr, rp->ai_addrlen, host, BUF_SIZE, serv, BUF_SIZE, NI_NUMERICHOST) == -1){
    //     perror("getnameinfo\n");
    // }
    // printf("%d bytes de %s\n", host);

    return 0;
}
