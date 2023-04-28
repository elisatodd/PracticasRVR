#include <string.h>

#include "Serializable.h"
#include "Socket.h"

/// @brief Utiliza getaddrinfo para obtener la dirección IP y el puerto de destino
/// y luego establecer la conexión utilizando el socket UDP. Primero, se inicializa
/// la estructura addrinfo con la familia de direcciones y el tipo de socket que se va a utilizar. 
/// A continuación, se llama a getaddrinfo para obtener una lista de direcciones de red que 
/// se corresponden con la dirección y el puerto especificados.
/// A continuación, se itera por esta lista de direcciones hasta encontrar una que pueda conectarse. 
/// Una vez que se establece la conexión, los miembros sd, sa, y sa_len se inicializan con los valores
/// de la dirección y el puerto que se han utilizado para establecer la conexión.
/// Si no se puede establecer una conexión, se muestra un mensaje de error y se cierra el socket.
/// @param address 
/// @param port 
Socket::Socket(const char * address, const char * port) : sd(-1) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* IPv4 */
    hints.ai_socktype = SOCK_DGRAM; /* UDP socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;           /* Any protocol */

    int s = getaddrinfo(address, port, &hints, &result);
    if (s != 0) {
        cerr << "getaddrinfo: " << gai_strerror(s) << endl;
        return;
    }

    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        sd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sd == -1)
            continue;

        if (connect(sd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */

        close(sd);
    }

    if (rp == nullptr) {            /* No address succeeded */
        cerr << "Could not connect" << endl;
        return;
    }

    sa = *rp->ai_addr;
    sa_len = rp->ai_addrlen;

    freeaddrinfo(result);           /* No longer needed */
}


int Socket::recv(Serializable &obj, Socket * &sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if ( bytes <= 0 )
    {
        return -1;
    }

    if ( sock != 0 )
    {
        sock = new Socket(&sa, sa_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    //Serializar el objeto
    obj.to_bin();
    char* serialized = obj.data();
    //Enviar el objeto binario a sock usando el socket sd 
    int bytes_sent = sendto(sd, serialized, obj.size(), 0, (struct sockaddr*) &sa, sa_len);
    if (bytes_sent < 0) {
        perror("Error sending binary object");
        return -1;
    }

    return bytes_sent;
}

bool operator== (const Socket &s1, const Socket &s2) {
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiereº
    const sockaddr_in* sin1 = reinterpret_cast<const sockaddr_in*>(&s1.sa);
    const sockaddr_in* sin2 = reinterpret_cast<const sockaddr_in*>(&s2.sa);
    return sin1->sin_family == sin2->sin_family &&
           sin1->sin_addr.s_addr == sin2->sin_addr.s_addr &&
           sin1->sin_port == sin2->sin_port;
}

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};

