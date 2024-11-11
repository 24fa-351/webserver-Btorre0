#include "http_message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

http_client_message_t *parse_http_client_message(const char *message) {
    http_client_message_t *parsed_message =
        malloc(sizeof(http_client_message_t));
    parsed_message->method = malloc(16);
    parsed_message->path = malloc(256);
    parsed_message->http_version = malloc(16);
    parsed_message->body = NULL;
    parsed_message->body_length = 0;

    sscanf(message, "%s %s %s", parsed_message->method, parsed_message->path,
           parsed_message->http_version);

    char *body_start = strstr(message, "\r\n\r\n");
    if (body_start != NULL) {
        body_start += 4;
        parsed_message->body_length = strlen(body_start);
        parsed_message->body = malloc(parsed_message->body_length + 1);
        strcpy(parsed_message->body, body_start);
    }

    return parsed_message;
}

void free_http_client_message(http_client_message_t *message) {
    free(message->method);
    free(message->path);
    free(message->http_version);
    if (message->body != NULL) {
        free(message->body);
    }
    free(message);
}

http_server_message_t *create_http_server_message(const char *http_version,
                                                  int status_code,
                                                  const char *status_text,
                                                  const char *body) {
    http_server_message_t *new_message = malloc(sizeof(http_server_message_t));
    new_message->http_version = malloc(16);
    new_message->status_text = malloc(256);
    new_message->body = NULL;
    new_message->body_length = 0;

    strcpy(new_message->http_version, http_version);
    new_message->status_code = status_code;
    strcpy(new_message->status_text, status_text);

    if (body != NULL) {
        new_message->body_length = strlen(body);
        new_message->body = malloc(new_message->body_length + 1);
        strcpy(new_message->body, body);
    }

    return new_message;
}

void free_http_server_message(http_server_message_t *message) {
    free(message->http_version);
    free(message->status_text);
    if (message->body != NULL) {
        free(message->body);
    }
    free(message);
}

char *serialize_http_server_message(http_server_message_t *message) {
    char *serialized_message = malloc(1024);
    sprintf(serialized_message, "%s %d %s\r\nContent-Length: %d\r\n\r\n%s",
            message->http_version, message->status_code, message->status_text,
            message->body_length, message->body);
    return serialized_message;
}