#include "controller.h"

#include "cmd.h"
#include "log.h"
#include "message.h"
#include "service.h"
#include "session.h"
#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <ui_controller.h>

#include "group.h"
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
gboolean show_home_window_callback(gpointer data);


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

    } else {
      log_message(ERROR, "Service is NULL");
    }
    break;
  case LOGIN:
      handle_login(self, message);
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
    get_chat_connected(self,message);
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

static bool validate_controller_and_message(Controller *controller, Message *msg, const char *function_name) {
    if (controller == NULL || controller->client == NULL) {
        log_message(WARN, "Invalid controller or client in %s", function_name);
        return false;
    }

    if (msg == NULL) {
        log_message(WARN, "Invalid message in %s", function_name);
        return false;
    }


    msg->position = 0;
    return true;
}


static char* read_error_message(Message *msg) {
    char *error_msg = malloc(256);
    if (error_msg == NULL) {
        log_message(ERROR, "Failed to allocate memory for error message");
        return NULL;
    }

    if (!message_read_string(msg, error_msg, 256)) {
        strcpy(error_msg, "Unknown error");
    }

    return error_msg;
}

void handle_login(Controller *self, Message *msg) {
    if (!validate_controller_and_message(self, msg, __func__)) {
        return;
    }

    Session *session = (Session *)self->client;
    bool login_ok = message_read_bool(msg);
    log_message(INFO, "Login result: %s", login_ok ? "success" : "failed");

    if (login_ok) {
        int user_id = (int)message_read_int(msg);
        if (user_id <= 0) {
            log_message(WARN, "Invalid user ID: %d", user_id);
            return;
        }

        char username[256] = {0};
        if (!message_read_string(msg, username, sizeof(username))) {
            return;
        }

        log_message(INFO, "Logged in as: %s (ID: %d)", username, user_id);


        User *user = createUser(NULL, session, username, "");
        if (user == NULL) {
            show_notification_window(ERROR, "Internal error: Failed to create user");
            return;
        }


        user->id = user_id;
        user->isOnline = true;
        user->session = session;
        session->user = user;

        on_show_ui(MAIN_UI_LEVEL_HOME);
    } else {
        char *error = read_error_message(msg);
        if (error) {
            show_notification_window(ERROR, error);
            free(error);
        } else {
            show_notification_window(ERROR, "Unknown login error");
        }
    }
}

void handle_logout(Controller *self, Message *msg) {
    if (!validate_controller_and_message(self, msg, __func__)) {
        return;
    }
    log_message(INFO, "Logout Result");
    Session *session = (Session *)self->client;
    msg->position = 0;
    bool logout_ok = message_read_bool(msg);
    if (logout_ok) {

        session->isLogin = false;
        show_notification_window(INFO, "Logout success");
    } else {


        show_notification_window(INFO, "Logout success");
    }

    free(msg);
}

void handle_register(Controller *self, Message *msg) {
    if (!validate_controller_and_message(self, msg, __func__)) {
        return;
    }

    bool is_success = message_read_bool(msg);

    if (is_success) {
        printf("Register successful! You can now log in.\n");
    } else {
        char *error_msg = read_error_message(msg);
        if (error_msg) {
            printf("Register failed: %s\n", error_msg);
            free(error_msg);
        } else {
            printf("Register failed: Unknown error\n");
        }
    }
}

char **get_online_users(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        return NULL;
    }

    uint8_t count = message_read_int(message);
    char **users = calloc(count, sizeof(char *));
    if (users == NULL) {
        log_message(ERROR, "Failed to allocate memory for users array");
        return NULL;
    }

    bool allocation_error = false;
    for (int i = 0; i < count; i++) {
        users[i] = malloc(32 * sizeof(char));
        if (users[i] == NULL) {
            allocation_error = true;
            break;
        }

        if (!message_read_string(message, users[i], 32)) {
            log_message(WARN, "Failed to read username at index %d", i);
        }
    }


    if (allocation_error) {
        for (int i = 0; i < count; i++) {
            free(users[i]);
        }
        free(users);
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        log_message(INFO, "User %d: %s", i, users[i]);
    }

    return users;
}

void get_all_users(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        return;
    }


    if (!controller->client->connected) {
        log_message(ERROR, "Invalid connection");
        return;
    }


    int count = (int)message_read_int(message);
    log_message(INFO, "Received %d users", count);


    GString *user_list = g_string_new(NULL);
    if (user_list == NULL) {
        log_message(ERROR, "Failed to create string buffer");
        return;
    }


    int current_user_id = controller->client->user->id;
    log_message(INFO, "Current user ID: %d", current_user_id);


    int friends_added = 0;
    for (int i = 0; i < count; i++) {
        int user_id = (int)message_read_int(message);
        char username[256] = {0};

        if (!message_read_string(message, username, sizeof(username))) {
            log_message(WARN, "Failed to read username at index %d", i);
            continue;
        }

        bool is_online = message_read_bool(message);


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


    FriendListContext *fl_data = g_malloc(sizeof(FriendListContext));
    if (!fl_data) {
        log_message(ERROR, "Failed to allocate memory for friend list data");
        g_string_free(user_list, TRUE);
        return;
    }

    fl_data->friend_list = g_strdup(user_list->str);
    fl_data->session = controller->client;



    g_string_free(user_list, TRUE);
}

void get_joined_groups(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        return;
    }

    bool has_groups = message_read_bool(message);
    if (!has_groups) {
        log_message(INFO, "User is not a member of any groups");
        return;
    }

    int group_count = (int)message_read_int(message);
    log_message(INFO, "User is a member of %d groups", group_count);

    Group **groups = calloc(group_count, sizeof(Group *));
    if (!groups) {
        log_message(ERROR, "Failed to allocate memory for group list");
        return;
    }

    for (int i = 0; i < group_count; i++) {
        groups[i] = malloc(sizeof(Group));
        if (!groups[i]) {
            log_message(ERROR, "Failed to allocate memory for group %d", i);
            continue;
        }


        groups[i]->id = (int)message_read_int(message);
        if (!message_read_string(message, groups[i]->name, sizeof(groups[i]->name))) {
            log_message(ERROR, "Invalid group name");
        }

        groups[i]->created_at = (long)message_read_long(message);


        int owner_id = (int)message_read_int(message);
        char owner_name[256] = {0};

        if (!message_read_string(message, owner_name, sizeof(owner_name))) {
            log_message(ERROR, "Invalid owner name");
        }


        if (owner_id != -1) {
            groups[i]->created_by = malloc(sizeof(User));
            if (groups[i]->created_by == NULL) {
                log_message(ERROR, "Memory allocation failed for created_by");
                continue;
            }

            groups[i]->created_by->id = owner_id;
            strncpy(groups[i]->created_by->username, owner_name,
                   sizeof(groups[i]->created_by->username) - 1);
            groups[i]->created_by->username[sizeof(groups[i]->created_by->username) - 1] = '\0';
        } else {
            groups[i]->created_by = NULL;
        }

        log_message(INFO, "Group %d: %s, Created by %s (%d) at %ld",
                   groups[i]->id, groups[i]->name,
                   groups[i]->created_by ? groups[i]->created_by->username : "Unknown",
                   owner_id, groups[i]->created_at);
    }


    for (int i = 0; i < group_count; i++) {
        if (groups[i]) {
            free(groups[i]->created_by);
            free(groups[i]);
        }
    }
    free(groups);
}

void get_chat_connected(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        return;
    }

    bool success = message_read_bool(message);
    if (!success) {
        printf("No chat history found\n");
        free(message);
        return;
    }

    int count = (int)message_read_int(message);
    printf("Found %d chat histories:\n", count);

    for (int i = 0; i < count; i++) {
        int id = (int)message_read_int(message);

        char *chat_with = malloc(1024);
        char *last_message = malloc(1024);
        char *sender_name = malloc(1024);

        if (!chat_with || !last_message || !sender_name) {
            log_message(ERROR, "Memory allocation failed for chat history");
            free(chat_with);
            free(last_message);
            free(sender_name);
            continue;
        }

        if (!message_read_string(message, chat_with, 1024)) {
            log_message(WARN, "Failed to read chat_with");
        }

        long last_time = (long)message_read_long(message);

        if (!message_read_string(message, last_message, 1024)) {
            log_message(WARN, "Failed to read last_message");
        }

        int sender_id = (int)message_read_int(message);

        if (!message_read_string(message, sender_name, 1024)) {
            log_message(WARN, "Failed to read sender_name");
        }

        ChatMessage *m = malloc(sizeof(ChatMessage));
        if (!m) {
            log_message(ERROR, "Memory allocation failed for ChatMessage");
            free(chat_with);
            free(last_message);
            free(sender_name);
            continue;
        }

        m->sender_id = id < 0 ? -id : id;
        m->sender_name = strdup(sender_name);
        m->target_name = strdup(chat_with);
        m->content = strdup(last_message);
        m->timestamp = last_time;
        m->is_group_message = id < 0;

        on_update_history_contact(m);

        free(chat_with);
        free(last_message);
        free(sender_name);
    }
    free(message);
}

void handle_join_group(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        if (message) free(message);
        return;
    }

    bool joined = message_read_bool(message);

    if (joined) {
        int group_id = (int)message_read_int(message);
        int user_id = (int)message_read_int(message);
        char *group_name = malloc(1024);
        char *last_message = malloc(1024);
        char *sender_name = malloc(1024);
        if (!group_name || !last_message || !sender_name)
        {
            log_message(ERROR, "Memory allocation failed for join_group");
            free(group_name);
            free(last_message);
            free(sender_name);
            return;
        }
        if (!message_read_string(message, group_name, 1024) ||
            !message_read_string(message, sender_name, 1024) ||
            !message_read_string(message, last_message, 1024))
        {
            log_message(WARN, "Failed to read data");
            show_notification_window(WARN, "Failed to read data");
            return;
        }

        ChatMessage *m = malloc(sizeof(ChatMessage));
        if (!m)
        {
            log_message(ERROR, "Memory allocation failed for join_group");
            return;
        }

        m->content = strdup(last_message);
        m->sender_name = strdup(sender_name);
        m->target_name = strdup(group_name);
        m->sender_id = group_id;
        m-> is_group_message = true;
        on_update_history_contact(m);
    } else {
        char response_msg[256] = {0};
        if (!message_read_string(message, response_msg, sizeof(response_msg))) {
            free(message);
            return;
        }
        show_notification_window(WARN, response_msg);
        log_message(WARN, "Join group failed: %s", response_msg);
    }

    free(message);
}

void delete_group(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        if (message) free(message);
        return;
    }

    bool result = message_read_bool(message);

    if (result) {
        log_message(INFO, "Group deleted successfully");
        int group_id = (int)message_read_int(message);
        char *last_message = malloc(1024);
        if (!message_read_string(message, last_message, 1024))
        {
            log_message(WARN, "Failed to read data");
            show_notification_window(WARN, "Failed to read data");
            return;
        }
        show_notification_window(INFO, last_message);
        on_remove_contact(group_id < 0 ? group_id : -group_id);

    } else {
        char *error_message = malloc(1024);
        if (!error_message) {
            log_message(ERROR, "Memory allocation failed for error message");
            free(message);
            return;
        }

        if (!message_read_string(message, error_message, 1024)) {
            log_message(WARN, "Failed to read error message");
            strcpy(error_message, "Unknown error");
        }

        log_message(INFO, "Failed to delete group: %s", error_message);
        show_notification_window(INFO, "Failed to delete group: %s", error_message);
        free(error_message);
    }

    free(message);
}

void create_group(Controller *controller, Message *msg) {
    if (!validate_controller_and_message(controller, msg, __func__)) {
        if (msg) free(msg);
        return;
    }

    bool result = message_read_bool(msg);
    if (result) {
        int group_id = (int)message_read_int(msg);
        int user_id = (int)message_read_int(msg);
        char *group_name = malloc(1024);
        char *last_message = malloc(1024);
        char *sender_name = malloc(1024);
        if (!group_name || !last_message || !sender_name)
        {
            log_message(ERROR, "Memory allocation failed for join_group");
            free(group_name);
            free(last_message);
            free(sender_name);
            return;
        }
        if (!message_read_string(msg, group_name, 1024) ||
            !message_read_string(msg, sender_name, 1024) ||
            !message_read_string(msg, last_message, 1024))
        {
            log_message(WARN, "Failed to read data");
            show_notification_window(WARN, "Failed to read data");
            return;
        }

        ChatMessage *m = malloc(sizeof(ChatMessage));
        if (!m)
        {
            log_message(ERROR, "Memory allocation failed for join_group");
            return;
        }
        m->content = strdup(last_message);
        m->sender_name = strdup(sender_name);
        m->target_name = strdup(group_name);
        m->sender_id = group_id;
        m-> is_group_message = true;
        on_update_history_contact(m);
    } else {
        char *error_message = malloc(1024);
        if (!error_message) {
            log_message(ERROR, "Memory allocation failed for error message");
            free(msg);
            return;
        }

        if (!message_read_string(msg, error_message, 1024)) {
            log_message(WARN, "Failed to read error message");
            strcpy(error_message, "Unknown error");
        }

        show_notification_window(ERROR, "Failed to create group: %s", error_message);
        free(error_message);
    }

    free(msg);
}
void leave_group(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        return;
    }
    bool result = message_read_bool(message);
    if (!result) {
        char *error_message = malloc(1024);
        if (!error_message) {
            log_message(ERROR, "Memory allocation failed for error message");
            free(message);
            return;
        }
        if (!message_read_string(message, error_message, 1024)) {
            log_message(WARN, "Failed to read error message");
            strcpy(error_message, "Unknown error");
        }
        show_notification_window(ERROR, "Error: %s", error_message);
        free(error_message);
    } else {
        int group_id = (int)message_read_int(message);
        int user_id = (int)message_read_int(message);
        char *group_name = malloc(1024);
        char *last_message = malloc(1024);
        char *sender_name = malloc(1024);
        if (!group_name || !last_message || !sender_name) {
            log_message(ERROR, "Memory allocation failed for join_group");
            free(group_name);
            free(last_message);
            free(sender_name);
            free(message);
            return;
        }
        if (!message_read_string(message, group_name, 1024) ||
            !message_read_string(message, sender_name, 1024) ||
            !message_read_string(message, last_message, 1024)) {
            log_message(WARN, "Failed to read data");
            show_notification_window(WARN, "Failed to read data");
            free(group_name);
            free(last_message);
            free(sender_name);
            free(message);
            return;
        }
        if (user_id == main_session->user->id) {
            on_remove_contact(group_id < 0 ? group_id : -group_id);
        } else {
            ChatMessage *m = malloc(sizeof(ChatMessage));
            if (!m) {
                log_message(ERROR, "Memory allocation failed for join_group");
                free(group_name);
                free(last_message);
                free(sender_name);
                free(message);
                return;
            }
            log_message(INFO,"Leaving2");
            m->content = strdup(last_message);
            m->sender_name = strdup(sender_name);
            m->target_name = strdup(group_name);
            m->sender_id = group_id;
            m->is_group_message = true;


            if (!m->content || !m->sender_name || !m->target_name) {
                log_message(ERROR, "Failed to duplicate strings");
                free(m->content);
                free(m->sender_name);
                free(m->target_name);
                free(m);
                free(group_name);
                free(last_message);
                free(sender_name);
                free(message);
                return;
            }

            on_update_history_contact(m);
        }

        free(group_name);
        free(last_message);
        free(sender_name);
    }

    free(message);
}

void handle_search_users(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        if (message) free(message);
        return;
    }

    bool result = message_read_bool(message);
    if (!result) {
        char *error_message = malloc(1024);
        if (!error_message) {
            log_message(ERROR, "Memory allocation failed for error message");
            free(message);
            return;
        }

        if (!message_read_string(message, error_message, 1024)) {
            log_message(WARN, "Failed to read error message");
        }

        free(error_message);
        free(message);
        return;
    }

    int count = (int)message_read_int(message);
    User *results = calloc(count, sizeof(User));
    if (!results) {
        log_message(ERROR, "Memory allocation failed for search results");
        free(message);
        return;
    }

    for (int i = 0; i < count; i++) {
        results[i].id = (int)message_read_int(message);

        char username[256];
        if (message_read_string(message, username, sizeof(username))) {
            results[i].username = strdup(username);
        } else {
            results[i].username = NULL;
        }
    }

    UserListData *data = malloc(sizeof(UserListData));
    if (!data) {
        log_message(ERROR, "Memory allocation failed for user list data");
        for (int i = 0; i < count; i++) {
            free(results[i].username);
        }
        free(results);
        free(message);
        return;
    }

    data->count = count;
    data->users = results;
    on_update_search_user(data);

    free(message);
}

void handle_group_noti(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        if (message) free(message);
        return;
    }

    int group_id = (int)message_read_int(message);
    int user_id = (int)message_read_int(message);

    char *user_name = malloc(1024);
    char *noti_message = malloc(1024);
    char *group_name = malloc(1024);

    if (!user_name || !noti_message || !group_name) {
        log_message(ERROR, "Memory allocation failed");
        free(user_name);
        free(noti_message);
        free(group_name);
        free(message);
        return;
    }

    if (!message_read_string(message, user_name, 1024)) {
        log_message(WARN, "Failed to read user name");
    }

    if (!message_read_string(message, noti_message, 1024)) {
        log_message(WARN, "Failed to read notification message");
    }

    if (!message_read_string(message, group_name, 1024)) {
        log_message(WARN, "Failed to read group name");
    }

    show_notification_window(INFO, "User %d Group id: %d %s %s %s",
               user_id, group_id, user_name, noti_message, group_name);

    free(user_name);
    free(noti_message);
    free(group_name);
    free(message);
}

void receive_user_message(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        if (message) free(message);
        return;
    }

    int sender_id = (int)message_read_int(message);
    int user_id = (int)message_read_int(message);

    char content[1024] = {0};
    char user_name[1024] = {0};
    if (!message_read_string(message, user_name, sizeof(user_name))) {
        log_message(ERROR, "Failed to read username");
        free(message);
        return;
    }
    if (!message_read_string(message, content, sizeof(content))) {
        log_message(ERROR, "Failed to read message content");
        free(message);
        return;
    }
    ChatMessage *m = malloc(sizeof(ChatMessage));
    if (!m)
    {
        log_message(ERROR, "Memory allocation failed");
        free(message);
        return;
    }
    m->sender_id = sender_id;
    m->sender_name = strdup(user_name);
    m->target_name = strdup(user_name);
    m->content = strdup(content);

    m->timestamp = time(NULL) + 7 * 3600;
    m->is_group_message = false;
    m->noti_message = true;
    on_update_history_contact(m);
    log_message(INFO, "Received User ID: %d, Content: %s", user_id, content);
    free(message);
}

void receive_group_message(Controller *controller, Message *message) {
    if (!validate_controller_and_message(controller, message, __func__)) {
        if (message) free(message);
        return;
    }
    bool result = message_read_bool(message);
    if (result)
    {
        int sender_id = (int)message_read_int(message);
        int group_id = (int)message_read_int(message);

        char content[1024] = {0};
        char group_name[1024] = {0};
        char sender_name[1024] = {0};
        if (!message_read_string(message, group_name, sizeof(group_name))) {
            log_message(ERROR, "Failed to read group name");
            free(message);
            return;
        }
        if (!message_read_string(message, sender_name, sizeof(sender_name))) {
            log_message(ERROR, "Failed to read sender name");
            free(message);
            return;
        }
        if (!message_read_string(message, content, sizeof(content))) {
            log_message(ERROR, "Failed to read message content");
            free(message);
            return;
        }
        ChatMessage *m = malloc(sizeof(ChatMessage));
        if (!m)
        {
            log_message(ERROR, "Memory allocation failed");
            free(message);
            return;
        }
        m->content = strdup(content);
        m->sender_id = group_id;
        m->sender_name = strdup(sender_name);
        m->target_name = strdup(group_name);
        m->timestamp = time(NULL) + 7 * 3600;
        m->is_group_message = true;
        m->noti_message = true;
        on_update_history_contact(m);
        log_message(INFO, "Received Group ID: %d, Content: %s", group_id, content);
    }

    free(message);
}

void get_user_message(Controller *controller, Message *msg) {
    if (!validate_controller_and_message(controller, msg, __func__)) {
        if (msg) free(msg);
        return;
    }

    log_message(INFO, "Get history user message");

    bool has_messages = message_read_bool(msg);
    if (!has_messages) {
        log_message(INFO, "No messages found");
        free(msg);
        return;
    }

    int count = (int)message_read_int(msg);
    log_message(INFO, "Received %d messages", count);

    ChatMessage *history = calloc(count, sizeof(ChatMessage));
    if (!history) {
        log_message(ERROR, "Memory allocation failed for chat history");
        free(msg);
        return;
    }

    for (int i = 0; i < count; i++) {
        int sender_id = (int)message_read_int(msg);

        char *sender_name = malloc(1024);
        char *content = malloc(1024);

        if (!sender_name || !content) {
            log_message(ERROR, "Memory allocation failed for message content");
            free(sender_name);
            free(content);
            continue;
        }

        if (!message_read_string(msg, sender_name, 1024)) {
            log_message(WARN, "Failed to read sender name");
        }

        if (!message_read_string(msg, content, 1024)) {
            log_message(WARN, "Failed to read message content");
        }

        long timestamp = (long)message_read_long(msg);
        log_message(INFO, "Timestamp %ld", timestamp);
        history[i].sender_id = sender_id;
        history[i].sender_name = strdup(sender_name);
        history[i].content = strdup(content);
        history[i].timestamp = timestamp;
        history[i].is_group_message = false;

        free(sender_name);
        free(content);
    }

    ChatMessageList *data = malloc(sizeof(ChatMessageList));
    if (!data) {
        log_message(ERROR, "Memory allocation failed for message list");

        for (int i = 0; i < count; i++) {
            free(history[i].sender_name);
            free(history[i].content);
        }
        free(history);
        free(msg);
        return;
    }

    data->history = history;
    data->count = count;
    on_load_history_message(data);

    free(msg);
}

void get_group_message(Controller *controller, Message *msg) {
    if (!validate_controller_and_message(controller, msg, __func__)) {
        if (msg) free(msg);
        return;
    }

    log_message(INFO, "Get history group message");

    bool has_messages = message_read_bool(msg);
    if (!has_messages) {
        log_message(INFO, "No group messages found");
        free(msg);
        return;
    }

    int count = (int)message_read_int(msg);
    log_message(INFO, "Received %d group messages", count);

    ChatMessage *history = calloc(count, sizeof(ChatMessage));
    if (!history) {
        log_message(ERROR, "Memory allocation failed for group chat history");
        free(msg);
        return;
    }

    for (int i = 0; i < count; i++) {
        int sender_id = (int)message_read_int(msg);

        char *sender_name = malloc(1024);
        char *content = malloc(1024);

        if (!sender_name || !content) {
            log_message(ERROR, "Memory allocation failed for message content");
            free(sender_name);
            free(content);
            continue;
        }

        if (!message_read_string(msg, sender_name, 1024)) {
            log_message(WARN, "Failed to read sender name");
        }

        if (!message_read_string(msg, content, 1024)) {
            log_message(WARN, "Failed to read message content");
        }

        long timestamp = (long)message_read_long(msg);

        history[i].sender_id = sender_id;
        history[i].sender_name = strdup(sender_name);
        history[i].content = strdup(content);
        history[i].timestamp = timestamp;
        history[i].is_group_message = true;

        free(sender_name);
        free(content);
    }

    ChatMessageList *data = malloc(sizeof(ChatMessageList));
    if (!data) {
        log_message(ERROR, "Memory allocation failed for group message list");
        for (int i = 0; i < count; i++) {
            free(history[i].sender_name);
            free(history[i].content);
        }
        free(history);
        free(msg);
        return;
    }

    data->history = history;
    data->count = count;
    on_load_history_message(data);

    free(msg);
}
