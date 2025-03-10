#include "controller.h"
#include "cmd.h"
#include "log.h"
#include "message.h"
#include "service.h"
#include "session.h"
#include "user.h"

void controller_on_message(Controller *self, Message *message);
void controller_on_connection_fail(Controller *self);
void controller_on_disconnected(Controller *self);
void controller_on_connect_ok(Controller *self);
void controller_message_in_chat(Controller *self, Message *ms);
void controller_message_not_in_chat(Controller *self, Message *ms);
void controller_new_message(Controller *self, Message *ms);

Controller *createController(Session *client)
{
  Controller *controller = (Controller *)malloc(sizeof(Controller));
  if (controller == NULL)
  {
    return NULL;
  }
  controller->client = client;
  controller->service = NULL;
  controller->user = NULL;
  controller->onMessage = controller_on_message;
  controller->onConnectionFail = controller_on_connection_fail;
  controller->onDisconnected = controller_on_disconnected;
  controller->onConnectOK = controller_on_connect_ok;
  controller->messageInChat = controller_message_in_chat;
  controller->messageNotInChat = controller_message_not_in_chat;
  controller->newMessage = controller_new_message;

  return controller;
}
void destroyController(Controller *controller)
{
  if (controller != NULL)
  {
    free(controller);
  }
}
void controller_set_service(Controller *controller, Service *service)
{
  if (controller != NULL)
  {
    controller->service = service;
  }
}
void controller_set_user(Controller *controller, User *user)
{
  if (controller != NULL)
  {
    controller->user = user;
    log_message(INFO, "Client %d: logged in successfully");
  }
}

void controller_on_message(Controller *self, Message *message)
{
  if (self == NULL || message == NULL)
  {
    log_message(ERROR, "Client %d: message is NULL");
    return;
  }

  uint8_t command = message->command;
  switch (command)
  {
  case LOGIN:
    log_message(INFO, "Client %d: login");
    break;
  case REGISTER:
    log_message(INFO, "Client %d: register");
    break;
  case LOGOUT:
    log_message(INFO, "Client %d: logout");
    break;
  case GET_SESSION_ID:
    log_message(INFO, "Client %d: get session id");
    break;
  case TRADE_KEY:
    log_message(INFO, "Client %d: trade key");
    break;
  default:
    log_message(ERROR, "Client %d: unknown command %d",
                command);
    break;
  }
}

void controller_on_connection_fail(Controller *self)
{
  if (self == NULL)
  {
    return;
  }
  log_message(ERROR, "Client %d: connection fail");
}

void controller_on_disconnected(Controller *self)
{
  if (self == NULL)
  {
    return;
  }
  log_message(WARN, "Disconnected");
}

void controller_on_connect_ok(Controller *self)
{
  if (self == NULL)
  {
    return;
  }
}

void controller_message_in_chat(Controller *self, Message *ms)
{
  if (self == NULL || ms == NULL)
  {
    return;
  }
  log_message(INFO, "Client %d: message in chat");
}

void controller_message_not_in_chat(Controller *self, Message *ms)
{
  if (self == NULL || ms == NULL)
  {
    return;
  }
  log_message(INFO, "Client %d: message not in chat");
}

void controller_new_message(Controller *self, Message *ms)
{
  if (self == NULL || ms == NULL)
  {
    return;
  }
  log_message(INFO, "Client %d: new message");
}
