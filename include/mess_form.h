#ifndef MESS_FORM_H
#define MESS_FORM_H

// Hàm hiển thị thông báo lỗi trong form
void show_message_form(const gchar *message, gboolean success) {
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, // Không cần window cha
        GTK_DIALOG_MODAL,
        success ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message
    );

    // Gắn callback khi nhấn OK
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);

    gtk_widget_show_all(dialog);
}

#endif //MESS_FORM_H
