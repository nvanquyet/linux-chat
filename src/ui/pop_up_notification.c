//
// Created by tranvanthuy on 4/10/25.
//
#include <gtk/gtk.h>
#include <ui_controller.h>

void show_message_form(const gchar *message, gboolean success) {
    // Tạo một cửa sổ thông báo modal, không có parent
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        success ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message
    );

    // Đặt tiêu đề cho cửa sổ
    gtk_window_set_title(GTK_WINDOW(dialog), success ? "Thông báo" : "Lỗi");

    // Gắn sự kiện nhấn nút OK → đóng dialog
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);

    // Hiển thị dialog
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
