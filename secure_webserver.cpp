#include <iostream>
#include <arpa/inet.h> 
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <syslog.h>

#define PORT 8080

void handle_client(int client_socket) {
    char buffer[1024] = {0};
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read < 0) {
        syslog(LOG_ERR, "Receive failed");
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';

    if (strncmp(buffer, "GET ", 4) != 0) {
        syslog(LOG_WARNING, "Invalid HTTP request");
        close(client_socket);
        return;
    }

    std::string message = "Hello, secure world!";
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: " 
             << message.length() << "\n\n" << message;

    send(client_socket, response.str().c_str(), response.str().length(), 0);
    
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        syslog(LOG_ERR, "Socket creation failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "Bind failed");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        syslog(LOG_ERR, "Listen failed");
        return 1;
    }

    while (true) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            syslog(LOG_ERR, "Accept failed");
            continue;
        }

        syslog(LOG_INFO, "Client connected: %s", inet_ntoa(address.sin_addr));
        handle_client(client_socket);
    }

    close(server_fd);
    return 0;
}
