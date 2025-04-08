#include "controller.h"

#include <chat_common.h>

#include "cmd.h"
#include "log.h"
#include "message.h"
#include "service.h"
#include "session.h"
#include "user.h"
#include <stdlib.h>
#include <string.h>

#include "group.h"
#include "mess_form.h"
void controller_on_message(Controller *self, Message *message);
void controller_on_connection_fail(Controller *self);
void controller_on_disconnected(Controller *self);
void controller_on_connect_ok(Controller *self);
void controller_message_in_chat(Controller *self, Message *ms);
void controller_message_not_in_chat(Controller *self, Message *ms);
void controller_new_message(Controller *self, Message *ms);
void handle_login(Controller *self, Message *msg);
void handle_logout(Controller *self, Message *msg);
void handle_register(Controller *self, Message *msg);
char **get_online_users(Controller *controller, Message *message);
gboolean show_chat_window_callback(gpointer data);


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
  case GET_USERS:
    if(self->service != NULL) {
      get_all_users(self, message);
      //get_online_users(self, message);
    } else {
      log_message(ERROR, "Service is NULL");
    }
    break;
  case LOGIN:
    log_message(INFO,"login");
    if(self->service != NULL) {
      handle_login(self, message);
    } else {
      log_message(ERROR, "Service is NULL");
    }
    break;
  case LOGOUT:
    handle_logout(self, message);
    break;
  case REGISTER:
    handle_register(self, message);
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
  case JOIN_GROUP:
    handle_join_group(self, message);
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
  case GET_USERS_MESSAGE:
    get_user_message(self,message);
    break;
  case GET_GROUPS_MESSAGE:
    get_group_message(self,message);
    break;
  case GROUP_NOTIFICATION:
    handle_group_noti(self, message);
    break;
  case SEARCH_USERS:
    handle_search_users(self, message);
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


int  current_user_id;
void handle_login(Controller *self, Message *msg) {
  // Validate pointers first
  if (self == NULL || self->client == NULL) {
    log_message(ERROR, "Invalid controller or client in handle_login");
    return;
  }

  Session *session = (Session *)self->client;

  if (msg == NULL) {
    // If message is empty, display error message
    show_message_form("Login Null", FALSE);
    return;
  }

  // Reset message position to read from beginning
  msg->position = 0;
  bool loginOk = message_read_bool(msg);
  log_message(INFO, "Login result: %s", loginOk ? "success" : "failed");

  if (loginOk) {
    int user_id = (int)message_read_int(msg);
    char username[256] = {0};

    if (!message_read_string(msg, username, sizeof(username))) {
      log_message(WARN, "Invalid username");
      show_message_form("Invalid username data", FALSE);
      return;
    }

    log_message(INFO, "Logged in as: %s (ID: %d)", username, user_id);

    // Create user object
    User *user = createUser(NULL, session, username, "");
    if (user == NULL) {
      log_message(ERROR, "Failed to create user object");
      show_message_form("Internal error: Failed to create user", FALSE);
      return;
    }

    // Update user and session properties
    user->id = user_id;
    user->isOnline = true;
    user->session = session;

    // Update session properties
    session->user = user;
    session->current_user_id = user_id;

    // Hide the login window if it exists
    if (session->loginWindow != NULL) {
      gtk_widget_hide(session->loginWindow);
      log_message(INFO, "Login window hidden");
    }

    // Use g_idle_add to display chat window on GTK main thread
    g_idle_add(show_chat_window_callback, session);

    log_message(INFO, "Login successful, showing chat window");
  } else {
    // Handle login failure
    char error[256] = {0};

    if (!message_read_string(msg, error, sizeof(error))) {
      log_message(ERROR, "Failed to read error message");
      strcpy(error, "Unknown login error");
    }

    log_message(INFO, "Login failed: %s", error);
    // Display server error message
    show_message_form(error, FALSE);
  }
}
void handle_logout(Controller *self, Message *msg) {
  log_message(INFO, "handle_logout invoked");
  Session *session = (Session *)self;

  if (self == NULL || self->client == NULL) {
    log_message(ERROR, "Invalid controller or client in handle_logout");
    return;
  }
  if (msg == NULL) {
    show_message_form("Logout message null", FALSE);
    return;
  }

  msg->position = 0;
  bool logoutOk = message_read_bool(msg);
  if (logoutOk) {
    // Nếu có thông tin user, giải phóng nó (và tránh truy cập sau khi free)
    if (session->user != NULL) {
      log_message(INFO, "User %s (ID: %d) logged out", session->user->username, session->user->id);
      free(session->user);
      session->user = NULL;
    }
    session->isLogin = FALSE;
    session->current_user_id = -1;

    // Ẩn main window nếu chưa bị ẩn
    if (main_window != NULL) {
      gtk_widget_hide(main_window);
    }

    // Hiển thị thông báo logout thành công và hiện lại cửa sổ login
    show_message_form("Logout success", TRUE);
    g_idle_add(show_login_window_callback, session);
  } else {
    char error[256] = {0};
    if (!message_read_string(msg, error, sizeof(error))) {
      log_message(ERROR, "Failed to read logout error message");
      return;
    }
    log_message(ERROR, "Logout failed: %s", error);
    show_message_form(error, FALSE);
  }

  // Giải phóng message nếu cần (sử dụng hàm message_free nếu có)
 // message_free(msg);
}





void handle_register(Controller *self, Message *msg) {
  if (msg == NULL) {
    log_message(ERROR, "Register response message is NULL");
    return;
  }

  msg->position = 0;

  bool isSuccess = message_read_bool(msg);

  if (isSuccess) {
    printf("Register successful! You can now log in.\n");
  } else {
    char errorMsg[256] = {0};
    if (message_read_string(msg, errorMsg, sizeof(errorMsg))) {
      printf("Register failed: %s\n", errorMsg);
    } else {
      printf("Register failed: Unknown error\n");
    }
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

// void get_all_users(Controller *controller, Message *message) {
//   if (!controller || !message) return;
//
//   message->position = 0; // Reset position để đọc từ đầu
//   int count = message_read_int(message);
//
//   printf("Danh sách tất cả người dùng (Tổng: %d):\n", count);
//   for (int i = 0; i < count; i++) {
//     int user_id = (int) message_read_int(message);
//     char username[1024];
//     // Đọc username và giới hạn buffer để tránh overflow
//     if (!message_read_string(message, username, sizeof(username))) {
//       printf("[Lỗi] Không đọc được username ở user %d\n", i);
//       break; // Dừng nếu không thể đọc
//     }
//
//     bool isOnline = message_read_bool(message);
//
//     printf("Username: %-20s (Id: %d) - Trạng thái: %s\n",
//            username, user_id,
//            isOnline ? "Online" : "Offline");
//   }
// }
void get_all_users(Controller *controller, Message *message) {
  if (!controller || !message) {
    log_message(ERROR, "Invalid controller or message");
    return;
  }

  // Check if the session is valid before proceeding
  if (!controller->client || !controller->client->connected) {
    log_message(ERROR, "Invalid session or connection");
    return;
  }

  message->position = 0;

  // Đọc số lượng người dùng từ tin nhắn
  int count = (int) message_read_int(message);
  log_message(INFO, "Received %d users", count);

  // Create a dynamic string to store user list
  GString *user_list = g_string_new(NULL);

  // Get current user ID for comparison
  int current_user_id = controller->client->current_user_id;
  log_message(INFO, "Current user ID: %d", current_user_id);

  // Process list of users
  int friends_added = 0;
  for (int i = 0; i < count; i++) {
    int user_id = (int) message_read_int(message);
    char username[2024];
    if (!message_read_string(message, username, sizeof(username))) {
      log_message(WARN, "Failed to read username at index %d", i);
      continue;
    }

    bool isOnline = message_read_bool(message);

    // Skip the current user - don't add them to the friend list
    if (user_id == current_user_id) {
      log_message(INFO, "Skipping current user: %s (ID: %d)", username, user_id);
      continue;
    }

    if (friends_added > 0) {
      g_string_append(user_list, ", ");
    }

    g_string_append_printf(user_list, "%s/%d", username, user_id);
    friends_added++;
  }

  // Prepare data for UI update
  FriendListData *fl_data = g_malloc(sizeof(FriendListData));

  if (!fl_data) {
    log_message(ERROR, "Failed to allocate memory for friend list data");
    g_string_free(user_list, TRUE);
    return;
  }

  fl_data->friend_list = g_strdup(user_list->str);
  fl_data->session = controller->client;

  // Update UI in the main thread
  //g_idle_add((GSourceFunc)update_friend_list, fl_data);
  //g_string_free(user_list, TRUE);
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
void handle_join_group(Controller *controller, Message *message) {
    if (!message) return;

    message->position = 0;

    bool joined = message_read_bool(message);

    char response_msg[256] = {0};
    if (!message_read_string(message, response_msg, sizeof(response_msg))) {
      log_message(ERROR, "Failed to read response message");
      return;
    }

    if (joined) {
      log_message(INFO, "Join group success: %s", response_msg);
    } else {
      log_message(WARN, "Join group failed: %s", response_msg);
    }
}
void delete_group(Controller *controller, Message *message) {
  if (message == NULL) {
    log_message(ERROR, "Invalid message received");
    return;
  }

  // Đọc kết quả thành công hay thất bại từ server
  bool result = message_read_bool(message);

  if (result) {
    // Nếu thành công, hiển thị thông báo thành công
    log_message(INFO, "Group deleted successfully.");
    printf("Group deleted successfully.\n");
  } else {
    // Nếu thất bại, đọc thông báo lỗi từ server
    char* error_message = (char*)malloc(1024);
    if (!message_read_string(message, error_message, 1024)) {
      log_message(WARN, "NULL Message");
    }
    log_message(INFO, "Failed to delete group: %s", error_message);
    printf("Failed to delete group: %s\n", error_message);
  }
}
void create_group(Controller *controller, Message *msg) {
  if (msg == NULL) {
    log_message(ERROR, "Invalid message received");
    return;
  }

  bool result = message_read_bool(msg);
  if (result) {
    int group_id = (int) message_read_int(msg);
    log_message(INFO, "Group created successfully with ID: %d", group_id);
    printf("Group created successfully. Group ID: %d\n", group_id);
  } else {
    log_message(INFO, "Failed to create group");
    printf("Failed to create group.\n");
  }
  free(msg);
}
void leave_group(Controller *controller, Message *message) {
  if (message == NULL) {
    log_message(ERROR, "Invalid message received");
    return;
  }

  // Đọc kết quả từ server (bool: thành công hay thất bại)
  bool result = message_read_bool(message);
  if (!result) {
    // Nếu không thành công, đọc thông báo lỗi
    char* error_message = (char*)malloc(1024);
    if (!message_read_string(message, error_message, 1024)) {
      log_message(WARN, "NULL Message");
    }
    printf("Error: %s\n", error_message);
    free(error_message);
  } else {
    log_message(INFO, "Successfully left the group");
    printf("You have successfully left the group.\n");
  }
  free(message);
}
void handle_search_users(Controller *controller, Message *message)
{
  if (message == NULL) {
    log_message(ERROR, "Invalid message received");
    return;
  }
  message->position = 0;
  bool result = message_read_bool(message);
  if (!result)
  {
    char* error_message = (char*)malloc(1024);
    if (!message_read_string(message, error_message, 1024))
    {
      log_message(WARN, "NULL Message");
    }
    return;
  }
  int count = (int)message_read_int(message);
  User *results = NULL;
  //loop to create data
  update_search_user_results(results, count);
}
void handle_group_noti(Controller *controller, Message *message) {
  if (message == NULL) {
    log_message(ERROR, "Invalid message received");
    return;
  }
  message->position = 0;
  int group_id = (int) message_read_int(message);
  int user_id = (int) message_read_int(message);
  char* noti_message = (char*)malloc(1024);
  char* user_name = (char*)malloc(1024);
  char* group_name = (char*)malloc(1024);
  if (!message_read_string(message, user_name, 1024)) {
    log_message(WARN, "NULL User");
  }
  if (!message_read_string(message, noti_message, 1024)) {
    log_message(WARN, "NULL Message");
  }
  if (!message_read_string(message, group_name, 1024)) {
    log_message(WARN, "NULL Group");
  }
  log_message(INFO, "User %d Group id: %d %s %s %s", user_id, group_id, user_name,noti_message, user_name);
  free(noti_message);
  free(message);
}

void receive_user_message(Controller *controller, Message *message) {
  message->position = 0;
  int sender_id = (int) message_read_int(message);
  int user_id = (int) message_read_int(message);
  char content[1024];
  if (!message_read_string(message, content, sizeof(content))) {
    log_message(ERROR, "Failed to read data");
    return;
  }
  log_message(INFO, "Received User ID: %d, Content: %s", user_id, content);
  free(message);
}
void receive_group_message(Controller *controller, Message *message) {
  log_message(INFO, "receive_group_message");
  message->position = 0;
  int sender_id = (int) message_read_int(message);
  int group_id = (int) message_read_int(message);
  char content[1024];
  if (!message_read_string(message, content, sizeof(content))) {
    log_message(ERROR, "Failed to read data");
    return;
  }
  log_message(INFO, "Received Group ID: %d, Content: %s", group_id, content);
  free(message);
}
void get_user_message(Controller *controller, Message* msg) {
  log_message(INFO, "Get history user message");
  if (msg == NULL) {
    log_message(ERROR, "Invalid message received from server");
    return;
  }

  msg->position = 0;

  bool has_messages = message_read_bool(msg);
  if (!has_messages) {
    log_message(INFO, "No messages found");
    return;
  }

  // Đọc số lượng tin nhắn
  int count = (int) message_read_int(msg);
  log_message(INFO, "Received %d messages", count);

  // Đọc và xử lý từng tin nhắn
  for (int i = 0; i < count; i++) {
    int sender_id = (int) message_read_int(msg);  // ID người gửi
    char* sender_name = (char*)malloc(1024);
    char* content = (char*)malloc(1024);
    if (!message_read_string(msg, sender_name, 1024)) {
      log_message(WARN, "NULL Message");
    }
    if (!message_read_string(msg, content, 1024)) {
      log_message(WARN, "NULL Message");
    }  // Nội dung tin nhắn
    long timestamp = (long) message_read_long(msg); // Thời gian gửi

    // Hiển thị thông tin tin nhắn
    printf("Message #%d\n", i + 1);
    printf("Sender ID: %d\n", sender_id);
    printf("Sender Name: %s\n", sender_name);
    printf("Content: %s\n", content);
    printf("Timestamp: %ld\n", timestamp);

    // Giải phóng bộ nhớ cho tên người gửi và nội dung tin nhắn nếu cần
    free(sender_name);
    free(content);
  }
  free(msg);
}
void get_group_message(Controller *controller, Message* msg) {
  if (msg == NULL) {
    log_message(ERROR, "Invalid message received from server");
    return;
  }

  msg->position = 0;

  // Kiểm tra xem có tin nhắn hay không
  bool has_messages = message_read_bool(msg);
  if (!has_messages) {
    log_message(INFO, "No messages found");
    return;
  }

  // Đọc số lượng tin nhắn
  int count = (int) message_read_int(msg);
  log_message(INFO, "Received %d group messages", count);

  // Đọc và xử lý từng tin nhắn
  for (int i = 0; i < count; i++) {
    int sender_id = (int) message_read_int(msg);  // ID người gửi
    char* sender_name = (char*)malloc(1024);
    char* content = (char*)malloc(1024);
    if (!message_read_string(msg, sender_name, 1024)) {
      log_message(WARN, "NULL Message");
    }
    if (!message_read_string(msg, content, 1024)) {
      log_message(WARN, "NULL Message");
    }  // Nội dung tin nhắn
    long timestamp = (long) message_read_long(msg); // Thời gian gửi

    // Hiển thị thông tin tin nhắn
    printf("Group Message #%d\n", i + 1);
    printf("Sender ID: %d\n", sender_id);
    printf("Sender Name: %s\n", sender_name);
    printf("Content: %s\n", content);
    printf("Timestamp: %ld\n", timestamp);

    // Giải phóng bộ nhớ cho tên người gửi và nội dung tin nhắn nếu cần
    free(sender_name);
    free(content);
  }
  free(msg);
}