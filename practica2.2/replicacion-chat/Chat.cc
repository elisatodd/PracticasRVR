#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data

    // Serializar el campo type
    char* tmp = _data;
    memcpy(tmp, &type, sizeof(uint8_t));
    tmp += sizeof(uint8_t);

    // Serializar el campo nick
    memcpy(tmp, nick.c_str(), nick.length() + 1);
    tmp += sizeof(char) * 8;

    // Serializar el campo message
    memcpy(tmp, message.c_str(), message.length() + 1);
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    // Copiar los datos del buffer al objeto ChatMessage
    char* tmp = bobj;

    // Copiar el campo type
    memcpy(&type, tmp, sizeof(uint8_t));
    tmp += sizeof(uint8_t);

    // Copiar el campo nick
    char nick_buffer[9];
    memcpy(nick_buffer, tmp, sizeof(char) * 8);
    nick_buffer[8] = '\0'; // asegurar que la cadena está terminada
    nick = std::string(nick_buffer);
    tmp += sizeof(char) * 8;

    // Copiar el campo message
    char message_buffer[81];
    memcpy(message_buffer, tmp, sizeof(char) * 80);
    message_buffer[80] = '\0'; // asegurar que la cadena está terminada
    message = std::string(message_buffer);

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{

    while (true) {
        /*
        * NOTA: los clientes están definidos con "smart pointers", es necesario
        * crear un unique_ptr con el objeto socket recibido y usar std::move
        * para añadirlo al vector
        */

        // Esperar a que llegue un mensaje de algún cliente
        Socket *client_socket = socket.recv();
        
        // Recibir mensaje
        std::string message;
        try {
            message = client_socket->recv_string();
        } catch (const std::exception &e) {
            // Si se produce un error al recibir el mensaje, se cierra la conexión con el cliente
            std::cout << "Error al recibir mensaje: " << e.what() << std::endl;
            remove_client(client_socket);
            continue;
        }

        // Determinar el tipo de mensaje recibido
        Message message_type = get_message_type(message);

        // Realizar acciones en función del tipo de mensaje recibido
        switch (message_type) {
            case Message::LOGIN: {
                // Añadir al vector clients
                add_client(client_socket);
                std::cout << "Nuevo cliente conectado." << std::endl;
                break;
            }
            case Message::LOGOUT: {
                // Eliminar del vector clients
                remove_client(client_socket);
                std::cout << "Cliente desconectado." << std::endl;
                break;
            }
            case Message::MESSAGE: {
                // Reenviar el mensaje a todos los clientes (menos el emisor)
                std::cout << "Mensaje recibido: " << message << std::endl;
                for (auto &client : clients) {
                    if (client.get() != client_socket) {
                        try {
                            client->send_string(message);
                        } catch (const std::exception &e) {
                            // Si se produce un error al enviar el mensaje, se cierra la conexión con el cliente
                            std::cout << "Error al enviar mensaje: " << e.what() << std::endl;
                            remove_client(client.get());
                        }
                    }
                }
                break;
            }
            case Message::UNKNOWN: {
                // Si se recibe un mensaje desconocido, se cierra la conexión con el cliente
                std::cout << "Mensaje desconocido recibido: " << message << std::endl;
                remove_client(client_socket);
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
    ChatMessage em(nick, "");
    em.type = ChatMessage::LOGOUT;

    socket.send(em, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        std::string msg;
        std::getline(std::cin, msg);

        ChatMessage em(nick, msg);
        em.type = ChatMessage::MESSAGE;

        // Enviar al servidor usando socket
        socket.send(em, socket);
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        ChatMessage msg;
        socket.recv(msg, socket);

        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        if (msg.nick != nick)
        {
            std::cout << msg.nick << ": " << msg.message << std::endl;
        }
        
    }
}

