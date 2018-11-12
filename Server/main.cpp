#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
 
#pragma comment (lib, "Ws2_32.lib")
 
#define IP_ADDRESS "192.168.56.1"
#define DEFAULT_PORT "3504"
#define DEFAULT_BUFLEN 512
 
struct client_type
{
    int id;
    SOCKET socket;
};
 
const char OPTION_VALUE = 1;
const int MAX_CLIENTS = 5;
 
//Function Prototypes
int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread);
int main();
 
int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread)
{
    std::string msg = "";
    char tempmsg[DEFAULT_BUFLEN] = "";
 
    //Session
    while (1)
    {
        memset(tempmsg, 0, DEFAULT_BUFLEN);
 
        if (new_client.socket != 0)
        {
            int iResult = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);
 
            if (iResult != SOCKET_ERROR)
            {
                if (strcmp("", tempmsg))
                    msg = "Client #" + std::to_string(new_client.id) + ": " + tempmsg;
 
                std::cout << msg.c_str() << std::endl;
 
                //Broadcast that message to the other clients
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (client_array[i].socket != INVALID_SOCKET)
                        if (new_client.id != i)
                            iResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
                }
            }
            else
            {
                msg = "Client #" + std::to_string(new_client.id) + " Disconnected";
 
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
 
    thread.detach();
 
    return 0;
}
