#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include <gtk/gtk.h>
#include <pango/pango.h>
#include <stdbool.h>
#include "session.h"
#include "log.h"
// Chat window management
#define CONTACT_MAX_LENGTH 64

// Main window reference
extern GtkWidget *main_window;

// FriendListData structure - Used to pass friend list data to GTK callbacks
typedef struct {
    gchar *friend_list;  // String containing the friend list
    Session *session;    // Pointer to the session
} FriendListData;

typedef struct {
    GtkWidget *window;
    GtkEntry *entry_username;
    GtkEntry *entry_password;
    Session *session;  // Session được truyền từ main
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
    Session *session;  // Session được truyền từ main
} LoginData;



// Function declarations for UI operations
void show_chat_window(Session *session);
gboolean show_chat_window_callback(gpointer data);
gboolean add_message_to_chat(gpointer data);
gboolean show_login_error_callback(gpointer data);
gboolean show_registration_success(gpointer data);
gboolean show_registration_error(gpointer data);
gboolean update_friend_list(gpointer data);

void *show_login_window(Session *session);
void show_message_form(const gchar *message, gboolean success);
void on_contact_clicked(GtkWidget *widget, gpointer data);
void show_create_group_window(Session *session);
void create_group_action(GtkWidget *widget, gpointer data) ;
void show_join_group_window(Session *session) ;
void show_register_window(Session *session);
gboolean add_message_to_chat(gpointer data) ;
void on_login_window_destroy(GtkWidget *widget, gpointer user_data);
gboolean show_login_window_callback(gpointer data);



#endif // CHAT_COMMON_H