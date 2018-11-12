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
    while (true)
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

int main()
{
    WSAData wsa_data;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    string sent_message = "";
    client_type client = { INVALID_SOCKET, -1, "" };
    int iResult = 0;
    string message;

    cout << "Starting Client...\n";

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (iResult != 0) {
        cout << "WSAStartup() failed with error: " << iResult << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    cout << "Connecting...\n";

    // Resolve the server address and port
    iResult = getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo() failed with error: " << iResult << endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        client.socket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (client.socket == INVALID_SOCKET) {
            cout << "socket() failed with error: " << WSAGetLastError() << endl;
            WSACleanup();
            system("pause");
            return 1;
        }

        // Connect to server.
        iResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client.socket);
            client.socket = INVALID_SOCKET;
            continue;
        }
        break;
    }