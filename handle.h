#ifndef HANDLE_H
#define HANDLE_H

#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// lecture
typedef struct {
    int client_sock;
    struct sockaddr_in client_addr;
} ClientInfo;

typedef struct {
    int request_count;
    long total_received_bytes;
    long total_sent_bytes;
    pthread_mutex_t stats_mutex;
} ServerStats;

// copilot +google
extern ServerStats server_stats;

void *handle_client(void *client_info);
void send_response(int client_sock, const char *header, const char *content);
void handle_static(int client_sock, const char *path);
void handle_stats(int client_sock);
void handle_calc(int client_sock, const char *query);
void handle_404(int client_sock);
void handle_error(int client_sock, const char *message);

#endif  // HANDLE_H