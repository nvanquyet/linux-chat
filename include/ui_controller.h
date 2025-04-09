#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <log.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include "session.h"
#include "chat_message.h"
#define CONTACT_MAX_LENGTH 64

// Main window reference
GtkWidget *current_ui;
Session *main_session;

typedef enum {
    LOGIN,
    REGISTER,
    HOME
} MAIN_UI_LEVEL;

typedef struct {
    gchar *friend_list;
    Session *session;
} FriendListContext;
typedef struct {
    GtkEntry *entry_username;
    GtkEntry *entry_password;
} CredentialForm;

typedef struct {
    int count;
    User *users;
} UserListData;

typedef struct {
    ChatMessage* history;
    int count;
} ChatMessageList;

//event
void on_show_ui(MAIN_UI_LEVEL level);

gboolean g_on_show_login_window(gpointer data);
gboolean g_on_show_register_window(gpointer data);
gboolean g_on_show_home_window(gpointer data);
gboolean g_on_show_create_window(gpointer data);
gboolean g_on_show_join_window(gpointer data);

gboolean g_on_update_search_user(gpointer data);
gboolean g_on_update_history_contact(gpointer data);
gboolean g_on_load_history_message(gpointer data);
gboolean g_on_receive_message(gpointer data);

// Function declarations for UI operations
//show ui
void show_chat_window(Session *session);
void init(Session *session);
void set_current_ui(GtkWidget *widget);
void show_home_window();
void show_login_window();
void show_create_group_window();
void show_join_group_window();
void show_register_window();
void show_notification_window(LogLevel level, const char *content, ...);


//home UI
void on_receive_message();
void on_load_history_message(ChatMessageList *data);
void on_update_history_contact(ChatMessage *data);
void on_update_search_user(UserListData *data);


void on_receive_message(int id, const char* message, bool isGroup);
#endif