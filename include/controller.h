#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "session.h"
#include "user.h"
#include "message.h"

typedef struct Controller Controller;
typedef struct Message Message;
typedef struct Session Session;
typedef struct User User;
typedef struct Service Service;

struct Controller {
    void (*onMessage)(Controller* self, Message* message);
    void (*onConnectionFail)(Controller* self);
    void (*onDisconnected)(Controller* self);
    void (*onConnectOK)(Controller* self);
    void (*messageInChat)(Controller* self, Message* ms);
    void (*messageNotInChat)(Controller* self, Message* ms);
    void (*newMessage)(Controller* self, Message* ms);
    
    Session* client;
    Service* service;
    User* user;
};

Controller* createController(Session* client);
void destroyController(Controller* controller);
void controller_set_service(Controller* controller, Service* service);
void controller_set_user(Controller* controller, User* user);
void controller_on_message(Controller* controller, Message* msg);


char **get_online_users(Controller* controller, Message* message);
void get_all_users(Controller *controller, Message *message);
void get_joined_groups(Controller *controller, Message *message);
void get_chat_history(Controller *controller, Message *message);

void get_user_message(Controller *controller, Message* msg);
void get_group_message(Controller *controller, Message* msg);

void delete_group(Controller *controller, Message *message);
void create_group(Controller *controller, Message *msg);
void leave_group(Controller *controller, Message *message);
void handle_group_noti(Controller *controller, Message *message);
void handle_join_group(Controller *controller, Message *message);


void receive_user_message(Controller *controller, Message *message);
void receive_group_message(Controller *controller, Message *message);
#endif