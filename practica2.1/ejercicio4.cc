#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

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
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];
		struct sockaddr client;
		socklen_t  clientlen = sizeof(struct sockaddr);
		int client_sd = accept(sd, (struct sockaddr*)&client, &clientlen);
		getnameinfo((struct sockaddr*)&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        printf("Conexión desde: %s [ %s ]\n", host, serv);

		while (1) {
			char buffer[1500];
			ssize_t bytes = recv(client_sd, buffer, 1499, 0);
			if (bytes == 0 || (buffer[0] == 'Q' && strlen(buffer) <= 2)) {
				printf("Conexión terminada\n");
				break;
			}
			buffer[bytes] = '\0';
            printf("\tMensaje enviado: %s", buffer);
			send(client_sd, buffer, bytes, 0);
		}
		close(client_sd);
	}

	return 0;
}