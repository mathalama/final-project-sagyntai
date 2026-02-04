#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define DEVICE_PATH "/dev/security_sensor"
#define PORT 9000
#define BUFFER_SIZE 256

int server_fd;

void handle_sigint(int sig) {
    printf("\nShutting down server...\n");
    close(server_fd);
    exit(0);
}

int main() {
    int client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    signal(SIGINT, handle_sigint);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Security Socket Server started on port %d\n", PORT);
    printf("Monitoring %s and broadcasting to clients...\n", DEVICE_PATH);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        int dev_fd = open(DEVICE_PATH, O_RDONLY);
        if (dev_fd < 0) {
            perror("Could not open device");
            const char *msg = "Error: Device not available\n";
            send(client_fd, msg, strlen(msg), 0);
        } else {
            // Read all current logs from the circular buffer and send to client
            while ((bytes_read = read(dev_fd, buffer, BUFFER_SIZE - 1)) > 0) {
                send(client_fd, buffer, bytes_read, 0);
            }
            close(dev_fd);
        }

        close(client_fd);
    }

    return 0;
}