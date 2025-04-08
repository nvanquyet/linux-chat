#ifndef SESSION_H
#define SESSION_H

#include "controller.h"
#include "message.h"
#include "service.h"
#include <stdbool.h>
#include <gtk/gtk.h>
// Forward declarations
typedef struct User User;
typedef struct Session Session;
typedef struct Controller Controller;
typedef struct Service Service;
typedef struct Message Message;

typedef unsigned char byte;

struct Session
{
  User *user;
  Controller *handler;
  Service *service;
  bool connected;
  bool connecting;
  bool clientOK;
  bool isLogin;
  char *ip;
  int port;
  int socket;
  bool isRunning;
  int  current_user_id;

  GtkWidget *loginWindow;
  GtkWidget *chatWindow;
  // Function pointers
  bool (*isConnected)(Session *self);
  void (*setHandler)(Session *self, Controller *handler);
  void (*setService)(Session *self, Service *service);
  void (*sendMessage)(Session *self, Message *message);
  void (*close)(Session *self);
  void (*clientOk)(Session *self);
  bool (*doSendMessage)(Session *self, Message *msg);
  void (*disconnect)(Session *self);
  void (*onMessage)(Session *self, Message *msg);
  void (*processMessage)(Session *self, Message *msg);
  Message *(*readMessage)(Session *self);
  void (*closeMessage)(Session *self);
  void (*doConnect)(Session *self, char *ip, int port);
  void (*connect)(Session *self, char *ip, int port);
  void (*initNetwork)(Session *self);

  // Opaque pointer for private implementation details
  void *_private;
  void *_key;
};

Session *createSession();
void destroySession(Session *session);
void session_set_handler(Session *session, Controller *handler);
void session_set_service(Session *session, Service *service);
void session_send_message(Session *session, Message *message);

#endif
