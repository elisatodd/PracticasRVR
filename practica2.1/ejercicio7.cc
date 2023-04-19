#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <pthread.h>

class ConnectionHandler {
public:
    ConnectionHandler(int client_sd) : m_client_sd(client_sd) {}
    void operator()() {
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];
        struct sockaddr client;
        socklen_t  clientlen = sizeof(struct sockaddr);
        getpeername(m_client_sd, &client, &clientlen);
        getnameinfo((struct sockaddr*)&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        printf("Conexión desde: %s [ %s ]\n", host, serv);

        while (1) {
            char buffer[1500];
            ssize_t bytes = recv(m_client_sd, buffer, 1499, 0);
            if (bytes == 0 || (buffer[0] == 'Q' && strlen(buffer) <= 2)) {
                printf("Conexión terminada\n");
                break;
            }
            buffer[bytes] = '\0';
            printf("\tMensaje enviado: %s", buffer);
            send(m_client_sd, buffer, bytes, 0);
        }

        close(m_client_sd);
    }
private:
    int m_client_sd;
};

void* connection_handler(void* arg) {
    int client_sd = *(int*)arg;
    ConnectionHandler handler(client_sd);
    handler();
    delete (int*)arg;
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    struct addrinfo hints;
    struct addrinfo* result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET; //ipv4
    hints.ai_socktype = SOCK_STREAM;
    if (argc != 3){
        fprintf(stderr, "Usage %s direction port \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int rc = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "Address info failure \n");
        exit(EXIT_FAILURE);
    }
    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    rc = bind(sd, result->ai_addr, result->ai_addrlen);
    if (rc == -1) {
        fprintf(stderr, "Binding failure \n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    listen(sd, 16);
    while (true) {
        struct sockaddr client;
        socklen_t  clientlen = sizeof(struct sockaddr);
        int client_sd = accept(sd, (struct sockaddr*)&client, &clientlen);
        int* arg = new int(client_sd);
        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_handler, (void*)arg) != 0) {
            perror("Error creating thread");
        } else {
            pthread_detach(thread);
        }
    }
    close(sd);
    return 0;
}
