#include "controller.h"
#include "cmd.h"
#include "log.h"
#include "message.h"
#include "service.h"
#include "session.h"
#include "user.h"
#include <stdlib.h>
#include <string.h>

#include "group.h"

void controller_on_message(Controller *self, Message *message);
void controller_on_connection_fail(Controller *self);
void controller_on_disconnected(Controller *self);
void controller_on_connect_ok(Controller *self);
void controller_message_in_chat(Controller *self, Message *ms);
void controller_message_not_in_chat(Controller *self, Message *ms);
void controller_new_message(Controller *self, Message *ms);

char **get_online_users(Controller *controller, Message *message);


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
  case SERVER_MESSAGE:
    if(self->service != NULL) {
      self->service->server_message(self->service, message);
    } else {
      log_message(ERROR, "Service is NULL");
    }
    break;
  case GET_ONLINE_USERS:
    if(self->service != NULL) {
      get_online_users(self, message);
    } else {
      log_message(ERROR, "Service is NULL");
    }
    break;
  case LOGIN_SUCCESS:
    self->client->isLogin = true;
    break;
  case GET_JOINED_GROUPS:
    get_joined_groups(self, message);
    break;
  case CREATE_GROUP:
    create_group(self, message);
    break;
  case LEAVE_GROUP:
    leave_group(self, message);
    break;
  case DELETE_GROUP:
    delete_group(self, message);
    break;
  case USER_MESSAGE:
    receive_user_message(self, message);
    break;
  case GROUP_MESSAGE:
    receive_group_message(self, message);
    break;
  case GET_CHAT_HISTORY:
    get_chat_history(self,message);
    break;
  default:
    log_message(ERROR, "Unknown command %d",
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
}

void controller_message_not_in_chat(Controller *self, Message *ms)
{
  if (self == NULL || ms == NULL)
  {
    return;
  }
}

void controller_new_message(Controller *self, Message *ms)
{
  if (self == NULL || ms == NULL)
  {
    return;
  }
}

char **get_online_users(Controller *controller, Message *message)
{
  if (controller == NULL || message == NULL)
  {
    return NULL;
  }
  
  uint8_t count = message_read_int(message);
  char **users = (char **)malloc(sizeof(char *) * count);
  if (users == NULL)
  {
    return NULL;
  }

  for (int i = 0; i < count; i++)
  {
    char *username = (char *)malloc(sizeof(char) * 32);
    if (username == NULL)
    {
      return NULL;
    }
    message_read_string(message, username, 32);
    users[i] = username;
  }

  for(int i = 0; i < count; i++) {
    log_message(INFO, "User %d: %s", i, users[i]);
  }
  return users;
}

void get_joined_groups(Controller *controller, Message *message) {
  if (!controller || !message) {
    log_message(ERROR, "Invalid controller or message");
    return;
  }

  bool has_groups = message_read_bool(message);
  if (!has_groups) {
    log_message(INFO, "User không tham gia nhóm nào");
    return;
  }

  int group_count = (int) message_read_int(message);
  log_message(INFO, "User tham gia %d nhóm", group_count);

  Group **groups = (Group **)malloc(sizeof(Group *) * group_count);
  if (!groups) {
    log_message(ERROR, "Không thể cấp phát bộ nhớ cho danh sách nhóm");
    return;
  }

  for (int i = 0; i < group_count; i++) {
    groups[i] = (Group *)malloc(sizeof(Group));  // Cấp phát bộ nhớ cho từng nhóm
    if (!groups[i]) {
      log_message(ERROR, "Không thể cấp phát bộ nhớ cho nhóm %d", i);
      continue;
    }
    // Đọc thông tin nhóm
    groups[i]->id = (int)message_read_int(message);
    if (!message_read_string(message, groups[i]->name, sizeof(groups[i]->name))) {
      log_message(ERROR, "Invalid name group");
    }

    groups[i]->created_at = (long)message_read_long(message);
    // Đọc owner id và name
    int owner_id = (int)message_read_int(message);
    char owner_name[256];

    if (!message_read_string(message, owner_name, sizeof(owner_name))) {
      log_message(ERROR, "Invalid owner");
    }
    // Nếu owner_id hợp lệ, cấp phát bộ nhớ cho created_by
    if (owner_id != -1) {
      groups[i]->created_by = (User *)malloc(sizeof(User));
      if (groups[i]->created_by == NULL) {
        log_message(ERROR, "Memory allocation failed for created_by.");
        continue;
      }
      groups[i]->created_by->id = owner_id;
      groups[i]->created_by->username = owner_name;
      groups[i]->created_by->username[sizeof(groups[i]->created_by->username) - 1] = '\0';

    } else {
      groups[i]->created_by = NULL;
    }

    log_message(INFO, "Nhóm %d: %s, Created by %s (%d) at %ld",
                groups[i]->id, groups[i]->name,
                groups[i]->created_by ? groups[i]->created_by->username : "Unknown",
                owner_id, groups[i]->created_at);
  }

  // Giải phóng bộ nhớ cho mỗi nhóm
  for (int i = 0; i < group_count; i++) {
    if (groups[i]->created_by != NULL) {
      free(groups[i]->created_by);
    }
    free(groups[i]);
  }
  free(groups);
}

void get_chat_history(Controller *controller, Message *message) {
  if (!controller || !message) {
    log_message(ERROR, "Invalid controller or message");
    return;
  }
  message->position = 0;
  bool success = message_read_bool(message);

  if (!success) {
    printf("Không có lịch sử chat nào\n");
    return;
  }

  int count = (int) message_read_int(message);
  printf("Tìm thấy %d lịch sử chat:\n", count);

  for (int i = 0; i < count; i++) {
    int id = (int) message_read_int(message);

    char* chat_with = (char*)malloc(1024);
    char* last_message = (char*)malloc(1024);
    char* sender_name = (char*)malloc(1024);

    if (!message_read_string(message, chat_with, 1024)) {
      log_message(WARN, "NULL Message");
    }
    long last_time = (long) message_read_long(message);
    if (!message_read_string(message, last_message, 1024)) {
      log_message(WARN, "NULL Message");
    }
    int sender_id = (int) message_read_int(message);  // Thêm trường sender_id
    if (!message_read_string(message, sender_name, 1024)) {
      log_message(WARN, "NULL sender_name");
    }
    // Hiển thị
    printf("===================\n");
    printf("Chat #%d:\n", id);
    printf("Sender: %s (ID: %d)\n", sender_name, sender_id);
    printf("Name: %s\n", chat_with);
    printf("Thời gian cuối: %ld\n", last_time);
    printf("Tin nhắn cuối: %s\n", last_message);
    printf("===================\n");

    free(chat_with);
    free(last_message);
  }
}

void delete_group(Controller *controller, Message *message) {
    log_message(INFO, "Delete Group");
}
void create_group(Controller *controller, Message *message) {
  log_message(INFO, "Create Group");
}
void leave_group(Controller *controller, Message *message) {
  log_message(INFO, "Leave Group");
}

void receive_user_message(Controller *controller, Message *message) {
  log_message(INFO, "receive_user_message");
}
void receive_group_message(Controller *controller, Message *message) {
 log_message(INFO, "receive_group_message");
}