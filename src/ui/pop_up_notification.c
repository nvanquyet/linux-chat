


#include <gtk/gtk.h>
#include <ui_controller.h>
gboolean g_on_show_notification(gpointer data);
void show_message_form(const gchar *message, gboolean success) {
    
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        success ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message
    );

    
    gtk_window_set_title(GTK_WINDOW(dialog), success ? "Thông báo" : "Lỗi");

    
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);

    
    gtk_widget_show_all(dialog);
}

gboolean g_on_show_notification(gpointer data)
{
    NotificationData *notif = (NotificationData *)data;
    show_message_form(notif->message, notif->success);
    g_free(notif->message);
    g_free(notif);
    return FALSE;
}
