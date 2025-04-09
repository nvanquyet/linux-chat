//
// Created by vawnwuyest on 09/04/2025.
//

#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H
#include <time.h>

typedef struct {
    int sender_id;
    char *sender_name;
    char *target_name;
    char *content;
    time_t timestamp;
    bool is_group_message;
} ChatMessage;
#endif //CHAT_MESSAGE_H
