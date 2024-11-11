#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#define BUFFER_SIZE 1024

// lecture
typedef struct {
    char *method;
    char *path;
    char *http_version;
    char *body;
    int body_length;
} http_client_message_t;

typedef struct {
    char *http_version;
    int status_code;
    char *status_text;
    char *body;
    int body_length;
} http_server_message_t;

// copilot
http_client_message_t *parse_http_client_message(const char *message);
void free_http_client_message(http_client_message_t *message);
http_server_message_t *create_http_server_message(const char *http_version,
                                                  int status_code,
                                                  const char *status_text,
                                                  const char *body);
void free_http_server_message(http_server_message_t *message);
char *serialize_http_server_message(http_server_message_t *message);

#endif  // HTTP_MESSAGE_H