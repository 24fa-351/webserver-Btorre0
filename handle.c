#include "handle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_message.h"

void *handle_client(void *client_info) {
    ClientInfo *info = (ClientInfo *)client_info;
    int sock = info->client_sock;

    char buffer[BUFFER_SIZE];
    int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        handle_error(sock, "Failed to receive request");
        free(client_info);
        return NULL;
    }

    buffer[bytes_received] = '\0';

    http_client_message_t *request = parse_http_client_message(buffer);

    if (strcmp(request->method, "GET") != 0) {
        handle_error(sock, "Only GET method is supported");
        free_http_client_message(request);
        free(client_info);
        return NULL;
    }

    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.request_count++;
    server_stats.total_received_bytes += bytes_received;
    pthread_mutex_unlock(&server_stats.stats_mutex);

    if (strncmp(request->path, "/static", 7) == 0) {
        handle_static(sock, request->path + 8);
    } else if (strncmp(request->path, "/stats", 6) == 0) {
        handle_stats(sock);
    } else if (strncmp(request->path, "/calc", 5) == 0) {
        handle_calc(sock, request->path + 6);
    } else {
        handle_404(sock);
    }

    free_http_client_message(request);
    close(sock);
    free(client_info);
    return NULL;
}

void send_response(int client_sock, const char *header, const char *content) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%s\r\nContent-Length: %lu\r\n\r\n%s", header,
            strlen(content), content);
    int bytes_sent = send(client_sock, buffer, strlen(buffer), 0);

    pthread_mutex_lock(&server_stats.stats_mutex);
    server_stats.total_sent_bytes += bytes_sent;
    pthread_mutex_unlock(&server_stats.stats_mutex);
}

// handling stats requests like /stats
void handle_stats(int client_sock) {
    pthread_mutex_lock(&server_stats.stats_mutex);
    char body[512];
    sprintf(body,
            "<html><body><h1>Server Stats</h1><p>Requests: %d</p><p>Received: "
            "%ld bytes</p><p>Sent: %ld bytes</p></body></html>",
            server_stats.request_count, server_stats.total_received_bytes,
            server_stats.total_sent_bytes);
    pthread_mutex_unlock(&server_stats.stats_mutex);

    http_server_message_t *response =
        create_http_server_message("HTTP/1.1", 200, "OK", body);
    char *serialized_response = serialize_http_server_message(response);

    send(client_sock, serialized_response, strlen(serialized_response), 0);

    free(serialized_response);
    free_http_server_message(response);
}

// handeling static files like /static/index.html
void handle_static(int client_sock, const char *path) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "static/%s", path);

    FILE *file = fopen(full_path, "rb");
    if (file == NULL) {
        handle_404(client_sock);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size);
    fread(file_content, 1, file_size, file);
    fclose(file);

    http_server_message_t *response =
        create_http_server_message("HTTP/1.1", 200, "OK", file_content);
    char *serialized_response = serialize_http_server_message(response);

    send(client_sock, serialized_response, strlen(serialized_response), 0);

    free(file_content);
    free(serialized_response);
    free_http_server_message(response);
}

// handling calc requests like /calc?a=1&b=2
void handle_calc(int client_sock, const char *query) {
    int a = 0, b = 0;
    char body[256];

    if (sscanf(query, "a=%d&b=%d", &a, &b) == 2) {
        sprintf(body,
                "<html><body><h1>Calculation Result</h1><p>%d + %d = "
                "%d</p></body></html>",
                a, b, a + b);
    } else {
        strcpy(body,
               "<html><body><h1>Error</h1><p>Invalid query parameters. Use "
               "format ?a=number&b=number.</p></body></html>");
    }

    http_server_message_t *response =
        create_http_server_message("HTTP/1.1", 200, "OK", body);
    char *serialized_response = serialize_http_server_message(response);

    send(client_sock, serialized_response, strlen(serialized_response), 0);

    free(serialized_response);
    free_http_server_message(response);
}

// 404 Not Found - The requested resource could not be found but may be
// available in the future.
void handle_404(int client_sock) {
    const char *body = "<html><body><h1>404 Not Found</h1></body></html>";
    http_server_message_t *response =
        create_http_server_message("HTTP/1.1", 404, "Not Found", body);
    char *serialized_response = serialize_http_server_message(response);

    send(client_sock, serialized_response, strlen(serialized_response), 0);

    free(serialized_response);
    free_http_server_message(response);
}

void handle_error(int client_sock, const char *message) {
    send_response(client_sock, "HTTP/1.1 400 Bad Request", message);
}