#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "session.h"
#include "chat_message.h"
#define CONTACT_MAX_LENGTH 64

// Main window reference
GtkWidget *current_ui;
Session *main_session;
typedef enum {
    INFO,
    WARN,
    ERROR,
    FATAL,
    DEBUG
} NOTI_LEVEL;


typedef enum {
    LOGIN,
    LOGOUT,
    HOME
} MAIN_UI_LEVEL;

typedef struct {
    gchar *friend_list;
    Session *session;
} FriendListData;

typedef struct {
    GtkWidget *window;
    GtkEntry *entry_username;
    GtkEntry *entry_password;
    Session *session;
} GroupData;

typedef struct {
    GtkWidget *window;
    Session *session;
} ChatApp;

typedef struct {
    Session *session;
    GtkWidget *name_entry;
    GtkWidget *pass_entry;
} CreateGroupData;

typedef struct {
    Session *session;
    GtkWidget *name_entry;
    GtkWidget *pass_entry;
} JoinGroupData;

typedef struct {
    GtkWidget *window;
    GtkEntry *entry_username;
    GtkEntry *entry_password;
    Session *session;
} LoginData;

typedef struct {
    int count;
    User *users;
} UerListData;

typedef struct {
    ChatMessage* history;
    int count;
} ChatMessageList;

//event
void on_show_ui(MAIN_UI_LEVEL level);

gboolean g_on_show_login_window(gpointer data);
gboolean g_on_show_logout_window(gpointer data);
gboolean g_on_show_home_window(gpointer data);
gboolean g_on_show_create_window(gpointer data);
gboolean g_on_show_join_window(gpointer data);
gboolean g_on_show_notification(gpointer data);
gboolean g_on_update_search_user(gpointer data);
gboolean g_on_update_history(gpointer data);
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
void show_logout_window();
void show_notification_window(NOTI_LEVEL level, const char *content, ...);
void on_receive_message();
void on_load_history_message();


gboolean show_chat_window_callback(gpointer data);
gboolean load_history_message(gpointer data);

void update_search_user_results(User *users, int count);
void append_chat_message(ChatMessage *msg);
void on_receive_message(int id, const char* message, bool isGroup);
void on_load_history(int id, const char *title, const char *message, long time, bool isGroup);
void load_chat_history(ChatMessage *history, int count);
#endif // CHAT_COMMON_H