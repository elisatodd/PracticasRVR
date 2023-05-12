#include "Chat.h"
#include <iostream>
#include <thread>

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <server> <port> <nick>" << std::endl;
        return 1;
    }

    ChatClient client(argv[1], argv[2], argv[3]);

    client.login();

    std::thread t_input(&ChatClient::input_thread, &client);
    std::thread t_net(&ChatClient::net_thread, &client);

    t_input.join();
    t_net.join();

    client.logout();
    
    return 0;
}
