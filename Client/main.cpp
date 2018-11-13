#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "3504"

struct client_type
{
    SOCKET socket;
    int id;
    char received_message[DEFAULT_BUFLEN];
};

int process_client(client_type &new_client);
int main();

int process_client(client_type &new_client) {
    while (true) {
        memset(new_client.received_message, 0, DEFAULT_BUFLEN);

        if (new_client.socket != 0) {
            int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);

            if (iResult != SOCKET_ERROR) {
                cout << new_client.received_message << endl;
            } else {
                cout << "recv() failed: " << WSAGetLastError() << endl;
                break;
            }
        }
    }

    if (WSAGetLastError() == WSAECONNRESET) {
        cout << "The server has disconnected" << endl;
    }
    return 0;
}

int main() {
    WSAData wsa_data;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    string sent_message;
    string temp_IP;
    client_type client = {INVALID_SOCKET, -1, ""};
    int iResult = 0;
    string message;

    // Ask the user to input the IP address they want to connect to.
    cout << "Starting Client...\n";
    cout << "Write the IP address you want to connect to:\n";
    getline(cin, temp_IP);
    LPCTSTR IP_ADDRESS = temp_IP.c_str();

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
    iResult = getaddrinfo(IP_ADDRESS, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo() failed with error: " << iResult << endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to a server and cout an error message in error.
        client.socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (client.socket == INVALID_SOCKET) {
            cout << "socket() failed with error: " << WSAGetLastError() << endl;
            WSACleanup();
            system("pause");
            return 1;
        }

        // Connect to server.
        iResult = connect(client.socket, ptr->ai_addr, (int) ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client.socket);
            client.socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    // Tell the user that the server could not be reached and then exit the program.
    if (client.socket == INVALID_SOCKET) {
        cout << "Unable to connect to server!" << endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    cout << "Successfully Connected" << endl;

    //Obtain id from server for this client;
    recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
    message = client.received_message;

    // If statement that checks if the server is full.
    if (message != "Server is full") {
        client.id = atoi(client.received_message);

        thread my_thread(process_client, client);

        // Loop that allows the user to send messages.
        bool userNameCheck = true;
        while (true) {
            // If statement that checks if a username has been given.
            if (userNameCheck) {
                // Loop that keeps asking for a username until a valid one has been provided.
                bool characterLimitLoop = true;
                while (characterLimitLoop){
                    cout << "Type your desired username: (max 20 characters)" << endl;
                    getline(cin, sent_message);
                    if (sent_message.length() < 21) {
                        iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);
                        userNameCheck = false;
                        characterLimitLoop = false;
                    } else {
                        cout << "Desired username is too long." << endl;
                    }
                }
                if (iResult <= 0) {
                    cout << "send() failed: " << WSAGetLastError() << endl;
                    break;
                }

            // Else statement that runs the chat messages after a username has ben given.
            } else {
                bool characterLimitLoop = true;
                while (characterLimitLoop) {
                    getline(cin, sent_message);
                    if (sent_message.length() < 140) {
                        iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);
                        characterLimitLoop = false;
                    } else {
                        cout << "Message is too long." << endl;
                    }
                }
                if (iResult <= 0) {
                    cout << "send() failed: " << WSAGetLastError() << endl;
                    break;
                }
            }
        }

        //Shutdown the connection since no more data will be sent
        my_thread.detach();

    // If the server is full this code runs.
    } else {
        cout << client.received_message << endl;
    }

    // Shuts down the socket and couts an error message if an error occurs.
    cout << "Shutting down socket..." << endl;
    iResult = shutdown(client.socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
        closesocket(client.socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    closesocket(client.socket);
    WSACleanup();
    system("pause");
    return 0;
}
