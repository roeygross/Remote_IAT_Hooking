// Final Assignment Of Operating Systems
// Roey Gross - 328494091 - Campus Lev

//server.cpp - The Server
//The server is always active and wait for client's connection
//The server is listening to IP 127.0.0.1
/*
The Implentation of the server is with WINAPI Sockets
To create the server we need to:
1. Initialize winsock
2. Create a socket
3. Bind the ip address and port to an socket
4. Tell winsock the socket is ready for listening
5. Wait for a connection
6. Close listening socket
7. Accept
8. Shutdown Winsock
9. Close the socket and cleanup
*/


#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <iostream>
using namespace std;
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096
#define DEFAULT_PORT "8000"

int main()
{
    cout << ">---SERVER---< " << endl << endl;
    while (1)
    {
        int iSendpInfo;
        char recvbuf[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;

        //1. Initialize winsock
        WSADATA wsaData;
        int ipInfo;

        //The WSAStartup function initiates use of the Winsock DLL by a process.
        ipInfo = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (ipInfo != 0) {
            cout << "error in WSAStartup, num: " << ipInfo << endl;
            return 1;
        }

        //2. Create a socket
        SOCKET serverSocket = INVALID_SOCKET;
        SOCKET clientSock = INVALID_SOCKET;

        //Initializing parameters
        struct addrinfo info;
        struct addrinfo* pInfo = &info;
        ZeroMemory(&info, sizeof(info));
        info.ai_family = AF_INET;
        info.ai_socktype = SOCK_STREAM;
        info.ai_protocol = IPPROTO_TCP;
        info.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        ipInfo = getaddrinfo(NULL, DEFAULT_PORT, &info, &pInfo);
        if (ipInfo != 0) {
            cout << "getaddrinfo failed with error: " << ipInfo << endl;
            WSACleanup();
            return 1;
        }

        // Create a SOCKET for connecting to server
        serverSocket = socket(pInfo->ai_family, pInfo->ai_socktype, pInfo->ai_protocol);
        if (serverSocket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError() << endl;
            freeaddrinfo(pInfo);
            WSACleanup();
            return 1;
        }

        //3. Bind the ip address and port to an socket 
        ipInfo = bind(serverSocket, pInfo->ai_addr, (int)pInfo->ai_addrlen);
        if (ipInfo == SOCKET_ERROR) {
            cout << "bind failed with error: " << WSAGetLastError() << endl;
            freeaddrinfo(pInfo);
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        freeaddrinfo(pInfo);

        //4. Tell winsock the socket is ready for listening
        ipInfo = listen(serverSocket, SOMAXCONN);
        if (ipInfo == SOCKET_ERROR) {
            cout << "listen failed with error: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }


        //5. Wait for a connection and connect to it after
        clientSock = accept(serverSocket, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            cout << "accept failed with error: " << WSAGetLastError() << endl;
            WSACleanup();
            return 1;
        }

        //6. Close listening socket
        closesocket(serverSocket);

        //7. ACCEPT
        ipInfo = recv(clientSock, recvbuf, recvbuflen, 0);
        cout << "CLIENT: "<< recvbuf << endl;

        //8. Shutdown Winsock
        ipInfo = shutdown(clientSock, SD_SEND);
        if (ipInfo == SOCKET_ERROR) {
            cout << "shutdown failed with error: " << WSAGetLastError() << endl;
            closesocket(clientSock);
            WSACleanup();
            return 1;
        }

        //9. Close the socket and cleanup
        closesocket(clientSock);
        WSACleanup();
    }
    return 0;
}