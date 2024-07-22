#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8000
#define MAXIMUM_REQUEST_SIZE 2048
#define ROOT "./src"

char* read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char*)malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }

    fclose(file);
    return content;
}

void handle_client(SOCKET clientSock) {
    char buffer[MAXIMUM_REQUEST_SIZE];
    int byteRecived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (byteRecived > 0) {
        buffer[byteRecived] = '\0';
        printf("Received request:\n%s\n", buffer);

        char filepath[256] = ROOT;
        if (strstr(buffer, "GET / ") != NULL) {
            strcat(filepath, "/index.html");
        } else if (strstr(buffer, "GET /script.js ") != NULL) {
            strcat(filepath, "/script.js");
        } else if (strstr(buffer, "GET /data ") != NULL) {
            const char *httpResponse = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 31\r\n"
                "\r\n"
                "Hello, world! ini dari bahasa C";
            send(clientSock, httpResponse, strlen(httpResponse), 0);
            closesocket(clientSock);
            return;
        } else {
            // Extract the requested file path from the request
            char *start = strstr(buffer, "GET /") + 5;
            char *end = strstr(start, " ");
            size_t path_length = end - start;
            strncat(filepath, start, path_length);
        }

        char *content = read_file(filepath);
        if (content) {
            const char *contentType = "text/html";
            if (strstr(filepath, ".js") != NULL) {
                contentType = "application/javascript";
            }

            char header[256];
            sprintf(header, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %zu\r\n"
                "\r\n", contentType, strlen(content));

            send(clientSock, header, strlen(header), 0);
            send(clientSock, content, strlen(content), 0);
            free(content);
        } else {
            const char *httpResponse = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 0\r\n"
                "\r\n";
            send(clientSock, httpResponse, strlen(httpResponse), 0);
        }
    }
    closesocket(clientSock);
}

int main() {
    WSADATA WSAData;
    SOCKET serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    if (listen(serverSock, 5) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSock == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            closesocket(serverSock);
            WSACleanup();
            return 1;
        }
        handle_client(clientSock);
    }

    closesocket(serverSock);
    WSACleanup();

    return 0;
}
