#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
     alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char* tmp=_data;
    memcpy(_data, &type, sizeof(uint8_t));
    tmp += nick.length() + 1;
    memcpy(_data, nick.c_str(), sizeof(char)*8);
    tmp += nick.length() + 1;
    memcpy(_data, message.c_str(), sizeof(char)*80);
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

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        Socket* clientSd;
        ChatMessage msg;
        //Recibir Mensajes en y en función del tipo de mensaje
        socket.recv(msg, clientSd);

        if(msg.type == ChatMessage::MessageType::LOGIN){
            std::cout << msg.nick << " LOGIN\n";
            clients.push_back(std::unique_ptr<Socket>(std::move(clientSd)));
        }
        // - LOGOUT: Eliminar del vector clients
        else if(msg.type == ChatMessage::MessageType::LOGOUT){
            std::cout << msg.nick << " LOGOUT\n";
            auto it = clients.begin();
            while(it != clients.end() && !(*(*it).get() == *clientSd)) it++;
            clients.erase(it);
        }
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        else if(msg.type == ChatMessage::MessageType::MESSAGE){
            std::cout << msg.nick << " MESSAGE\n";
            for(int i=0; i<clients.size(); ++i){
                if(!(*(clients[i].get()) == *clientSd))
                    socket.send(msg, (*clients[i].get()));
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

        if(msg=="q"){
            logout();
            break;
        }
        
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
        socket.recv(msg);
        
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        if (msg.nick != nick)
        {
            std::cout << msg.nick << ": " << msg.message << std::endl;
        }
        
    }
}

