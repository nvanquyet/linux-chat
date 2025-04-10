#include <gtk/gtk.h>
#include <glib.h>
#include <session.h>
#include <ui_controller.h>

#include "log.h"
#include "user.h"
extern Session* main_session;
extern GtkWidget* current_ui;
// Prototype các hàm
static void on_register_button_clicked(GtkWidget *button, gpointer user_data);
static void on_back_button_clicked(GtkWidget *button, gpointer user_data);
static void on_registration_window_destroy(GtkWidget *widget, gpointer user_data);
gboolean g_on_show_register_window(gpointer data);
void show_register_window();


// Xử lý nút "Đăng ký"
static void on_register_button_clicked(GtkWidget *button, gpointer user_data) {
    CredentialForm *reg_data = (CredentialForm *)user_data;
    const gchar *username = gtk_entry_get_text(reg_data->entry_username);
    const gchar *password = gtk_entry_get_text(reg_data->entry_password);
    const gchar *confirm  = gtk_entry_get_text(reg_data->entry_confirm);

    if (g_strcmp0(username, "") == 0 || g_strcmp0(password, "") == 0) {
        show_notification_window(INFO, "Please fill in all required fields!");
        return;
    }

    if (g_strcmp0(password, confirm) != 0) {
        show_notification_window(INFO, "Password confirmation does not match!");
        return;
    }

    User *user = createUser(NULL, main_session, username, password);
    if (user != NULL) {
        main_session->user = user;
        user->userRegister(user);
        on_show_ui(MAIN_UI_LEVEL_LOGIN);
    }
}


// Xử lý nút "Quay lại"
static void on_back_button_clicked(GtkWidget *button, gpointer user_data) {
    on_show_ui(MAIN_UI_LEVEL_LOGIN);
}

// Giải phóng bộ nhớ khi đóng cửa sổ
static void on_registration_window_destroy(GtkWidget *widget, gpointer user_data) {
    CredentialForm *reg_data = (CredentialForm *)user_data;
    g_free(reg_data);
    exit(0);
}
void create_registration_ui() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Register");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 250);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Allocate data for the registration form
    CredentialForm *reg_data = g_malloc(sizeof(CredentialForm));
    reg_data->entry_username = GTK_ENTRY(gtk_entry_new());
    reg_data->entry_password = GTK_ENTRY(gtk_entry_new());
    reg_data->entry_confirm = GTK_ENTRY(gtk_entry_new());

    // Hide password inputs
    gtk_entry_set_visibility(reg_data->entry_password, FALSE);
    gtk_entry_set_visibility(reg_data->entry_confirm, FALSE);

    // Main layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Username:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(reg_data->entry_username), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Password:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(reg_data->entry_password), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Confirm Password:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(reg_data->entry_confirm), FALSE, FALSE, 0);

    // Register button
    GtkWidget *btn_register = gtk_button_new_with_label("Register");
    g_signal_connect(btn_register, "clicked", G_CALLBACK(on_register_button_clicked), reg_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_register, FALSE, FALSE, 0);

    // Back to login button
    GtkWidget *btn_back = gtk_button_new_with_label("Back to Login");
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_button_clicked), reg_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_back, FALSE, FALSE, 0);

    // Handle window close
    g_signal_connect(window, "destroy", G_CALLBACK(force_exit), reg_data);

    // Show everything
    gtk_widget_show_all(window);

    set_current_ui(window); // If you have a function managing the current UI
}


gboolean g_on_show_register_window(gpointer data) {
    create_registration_ui();
    return false;
}