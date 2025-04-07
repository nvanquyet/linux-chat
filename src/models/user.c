#include "user.h"
#include "session.h"
#include "service.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include "cmd.h"
#include "group.h"
#include "message.h"

void login(User *self);
void logout(User *self);
void userRegister(User *self);

User *createUser(User *self, Session *client,const char *username, const char *password)
{
    if (self == NULL)
    {
        self = (User *)malloc(sizeof(User));
        if (self == NULL)
        {
            return NULL;
        }
    }

    self->id = 1;
    self->username = strdup(username);
    self->password = strdup(password);
    self->isOnline = false;
    self->session = client;
    self->service = NULL;
    self->ipAddr = NULL;
    self->lastLogin = 0;

    self->login = login;
    self->logout = logout;
    self->userRegister = userRegister;
    self->isCleaned = NULL;
    return self;
}

void destroyUser(User *user)
{
    if (user == NULL)
    {
        return;
    }

    if (user->username != NULL)
    {
        free(user->username);
    }

    if (user->password != NULL)
    {
        free(user->password);
    }

    if (user->ipAddr != NULL)
    {
        free(user->ipAddr);
    }

    free(user);
}

void user_set_session(User *user, Session *session)
{
    user->session = session;
}

void user_set_service(User *user, Service *service)
{
    user->service = service;
}

void login(User *self)
{
    if (self->isOnline)
    {
        log_message(INFO, "User is already online");
        return;
    }
    Message *msg = message_create(LOGIN);
    if (msg == NULL)
    {
        log_message(ERROR, "Failed to allocate memory for message");
        return;
    }

    message_write_string(msg, self->username);
    message_write_string(msg, self->password);
    session_send_message(self->session, msg);
}

void logout(User *self)
{
    self->isOnline = false;
    //send to server logout message
    Message *msg = message_create(LOGOUT);
    if (msg == NULL) {
        log_message(ERROR, "Failed to allocate memory for message");
        return;
    }
    session_send_message(self->session, msg);
}
void userRegister(User *self)
{
    self->isOnline = false;
    if(self->username == NULL || self->password == NULL) {
        log_message(ERROR, "Username or password is NULL");
        return;
    }
    log_message(INFO, "User registered");
    Message *msg = message_create(REGISTER);
    if (msg == NULL)
    {
        log_message(ERROR, "Failed to allocate memory for message");
        return;
    }

    message_write_string(msg, self->username);
    message_write_string(msg, self->password);
    log_message(INFO, "Registering with username: %s, password: %s", self->username, self->password);
    log_message(INFO, "SESSION IS  %s",self->session->connected ? "connected" : "disconnected");
    session_send_message(self->session, msg);
}