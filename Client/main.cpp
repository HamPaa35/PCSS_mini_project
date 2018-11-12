#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define IP_ADDRESS "192.168.56.1"
#define DEFAULT_PORT "3504"

struct client_type
{
    SOCKET socket;
    int id;
    char received_message[DEFAULT_BUFLEN];
};

int process_client(client_type &new_client);
int main();

int process_client(client_type &new_client)
{
    while (1)
    {
        memset(new_client.received_message, 0, DEFAULT_BUFLEN);

        if (new_client.socket != 0)
        {
            int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);

            if (iResult != SOCKET_ERROR)
                cout << new_client.received_message << endl;
            else
            {
                cout << "recv() failed: " << WSAGetLastError() << endl;
                break;
            }
        }
    }

    if (WSAGetLastError() == WSAECONNRESET)
        cout << "The server has disconnected" << endl;

    return 0;
}