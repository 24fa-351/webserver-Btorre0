#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "handle.h"
#include "http_message.h"

#define DEFAULT_PORT 80

ServerStats server_stats = {0, 0, 0, PTHREAD_MUTEX_INITIALIZER};

int main(int argc, char *argv[]) {
    int server_sock, port = DEFAULT_PORT;
    struct sockaddr_in server_addr;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&server_stats.stats_mutex, NULL);

    while (1) {
        ClientInfo *client_info = malloc(sizeof(ClientInfo));
        socklen_t addr_len = sizeof(client_info->client_addr);

        client_info->client_sock =
            accept(server_sock, (struct sockaddr *)&client_info->client_addr,
                   &addr_len);
        if (client_info->client_sock < 0) {
            perror("Accept failed");
            free(client_info);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client,
                           (void *)client_info) != 0) {
            perror("Thread creation failed");
            close(client_info->client_sock);
            free(client_info);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_sock);
    pthread_mutex_destroy(&server_stats.stats_mutex);
    return 0;
}