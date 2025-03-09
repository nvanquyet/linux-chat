#include "user.h"
#include "session.h"
#include "service.h"
#include "log.h"
#include <stdlib.h>

void login(User *self);
void logout(User *self);
void userRegister(User *self);

User *createUser(User *self, Session *client, char *username, char *password)
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
    self->isOnline = true;
}

void logout(User *self)
{
    self->isOnline = false;
}
void userRegister(User *self)
{
    self->isOnline = false;
}