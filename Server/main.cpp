#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
 
#pragma comment (lib, "Ws2_32.lib")

//Defining the port number and the buffer for the message length
#define DEFAULT_PORT "3504"
#define DEFAULT_BUFLEN 512

//A custom type to store the information of the client in
struct client_type
{
    int id;
    std::string name;
    bool nameState = false;
    SOCKET socket;
};


const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 10;
 
//Function decelerations for the program
int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread);
int main();

int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread)
{
    //temp variables for the code, to allocate memory
    std::string msg = "";
    char tempmsg[DEFAULT_BUFLEN] = "";
    //Variable to remember the client name, even when they have disconnected
    std::string clientName;
 
    //The loop for the chat session
    while (1)
    {
        //allocates memory for the tmp variables
        memset(tempmsg, 0, DEFAULT_BUFLEN);

        //Checks if the client has a name, and if not, sets their first message to be their name
        if(new_client.socket != 0 && !new_client.nameState){
            int iResult = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);
            new_client.name = tempmsg;
            clientName = tempmsg;
            new_client.nameState = true;
        }

        //The code for receiving normal messages, if the client has a name and a socket
        else if (new_client.socket != 0 && new_client.nameState)
        {
            //Receives the clients chat message
            int iResult = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);

            //Checks if the message is valid
            if (iResult != SOCKET_ERROR)
            {
                if (strcmp("", tempmsg))
                    msg = new_client.name + ": " + tempmsg;
 
                std::cout << msg.c_str() << std::endl;
 
                //Broadcast the message to all other clients
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (client_array[i].socket != INVALID_SOCKET)
                        if (new_client.id != i)
                            iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
                }
            }

            //If there is a socket error, the client will be disconnected, and the socket will be closed
            else
            {
                msg = clientName + " has Disconnected";
 
                std::cout << msg << std::endl;
 
                closesocket(new_client.socket);
                closesocket(client_array[new_client.id].socket);
                client_array[new_client.id].socket = INVALID_SOCKET;
 
                //Broadcast the disconnection message to the other clients
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (client_array[i].socket != INVALID_SOCKET)
                        iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
                }
 
                break;
            }
        }
    } //end while
 
    //The thread of the client is disconnected to free up threads
    thread.detach();
 
    return 0;
}

int main()
{
    WSADATA wsaData;
    struct addrinfo hints;
    struct addrinfo *server = NULL;
    SOCKET server_socket = INVALID_SOCKET;
    std::string msg = "";
    std::vector<client_type> client(MAX_CLIENTS);
    int num_clients = 0;
    int temp_id = -1;
    std::thread my_thread[MAX_CLIENTS];

    //Initialize Winsock
    std::cout << "Intializing Winsock..." << std::endl;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //Setup hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    //Setup Server
    std::cout << "Setting up server..." << std::endl;
    getaddrinfo(NULL, DEFAULT_PORT, &hints, &server);

    //Create a listening socket for connecting to server
    std::cout << "Creating server socket..." << std::endl;
    server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

    //Setup socket options
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
    setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

    //Assign an address to the server socket.
    std::cout << "Binding socket..." << std::endl;
    bind(server_socket, server->ai_addr, (int)server->ai_addrlen);

    //Listen for incoming connections.
    std::cout << "Listening..." << std::endl;
    listen(server_socket, SOMAXCONN);
    //Initialize the client list
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client[i] = { -1, client[i].name, false, INVALID_SOCKET };
    }
    while (1)
    {

        SOCKET incoming = INVALID_SOCKET;
        incoming = accept(server_socket, NULL, NULL);

        //If there is no valid socket, the loop wil be exited
        if (incoming == INVALID_SOCKET) continue;

        //Reset the number of clients
        num_clients = -1;

        //Create a temporary id for the next client
        temp_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client[i].socket == INVALID_SOCKET && temp_id == -1)
            {
                client[i].socket = incoming;
                client[i].id = i;
                temp_id = i;
            }

            if (client[i].socket != INVALID_SOCKET)
                num_clients++;

            std::cout << client[i].socket << std::endl;
        }

        if (temp_id != -1)
        {
            //Send the id to that client
            std::cout <<"New person has joined the chat" << std::endl;
            msg = std::to_string(client[temp_id].id);
            send(client[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

            //Create a thread process for that client
            my_thread[temp_id] = std::thread(process_client, std::ref(client[temp_id]), std::ref(client), std::ref(my_thread[temp_id]));
        }
        else
        {
            msg = "Server is full";
            send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
            std::cout << msg << std::endl;
        }
    } //end while


    //Close listening socket
    closesocket(server_socket);

    //Close client socket
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        my_thread[i].detach();
        closesocket(client[i].socket);
    }

    //Clean up Winsock
    WSACleanup();
    std::cout << "Program has ended successfully" << std::endl;

    system("pause");
    return 0;
}
