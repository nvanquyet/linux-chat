//
// Created by vawnwuyest on 09/04/2025.
//
#include "../include/ui_controller.h"

#include <log.h>

void on_show_ui(MAIN_UI_LEVEL level)
{
    switch (level)
    {
        case MAIN_UI_LEVEL_LOGIN:
            {
                show_login_window();
                break;
            }
        case MAIN_UI_LEVEL_REGISTER:
            {
                show_register_window();
                break;
            }
        case MAIN_UI_LEVEL_HOME:
            {
                show_home_window();
                break;
            }
        default:
            {
                show_notification_window(ERROR, "No has windows %s", level);
                break;
            }
    }
}


void set_current_ui(GtkWidget *widget) {
    if (widget == NULL) return;

    if (current_ui != NULL) {
        // Option 1: Hide
        gtk_widget_hide(current_ui);

        // // Option 2: Remove from parent's container
        // GtkWidget *parent = gtk_widget_get_parent(current_ui);
        // if (parent != NULL) {
        //     gtk_container_remove(GTK_CONTAINER(parent), current_ui);
        // }
        //// Option 3: Destroy widget
        //gtk_widget_destroy(current_ui);
    }

    current_ui = widget;
}

void show_home_window()
{
    g_idle_add((GSourceFunc)g_on_show_home_window, NULL);
}
void show_login_window()
{
    g_idle_add((GSourceFunc)g_on_show_login_window, NULL);

}
void show_create_group_window()
{
    g_idle_add((GSourceFunc)g_on_show_create_window, NULL);

}
void show_join_group_window()
{
    g_idle_add((GSourceFunc)g_on_show_join_window, NULL);
}
void show_register_window()
{
    g_idle_add((GSourceFunc)g_on_show_register_window, NULL);

}
void show_notification_window(LogLevel level, const char *content, ...)
{
    GtkMessageType type = level == ERROR ? GTK_MESSAGE_ERROR : (level == INFO ? GTK_MESSAGE_INFO : (level == WARN ? GTK_MESSAGE_WARNING : GTK_MESSAGE_OTHER));
    va_list args;
    va_start(args, content);
    vprintf(content, args);
    va_end(args);

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        "%s", content
    );

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
    gtk_widget_show_all(dialog);
}

// void on_receive_message()
// {
//
// }
void on_load_history_message(ChatMessageList *data)
{
    g_idle_add((GSourceFunc)g_on_load_history_message, data);
}
void on_update_history_contact(ChatMessage *data)
{
    g_idle_add((GSourceFunc)g_on_update_history_contact, data);
}
void on_remove_contact(int id)
{
    g_idle_add((GSourceFunc)g_on_remove_history_contact, GINT_TO_POINTER(id));
}
void on_update_search_user(UserListData *data)
{
    g_idle_add((GSourceFunc)g_on_update_search_user, data);
}