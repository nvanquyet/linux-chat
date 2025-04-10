#include <gtk/gtk.h>
#include <session.h>
#include <ui_controller.h>

#include "log.h"
#include "user.h"
#include "session.h"
extern Session* main_session;
extern GtkWidget* current_ui;

static void on_login_button_clicked(GtkWidget *button, gpointer user_data);
static void on_register_button_clicked(GtkWidget *button, gpointer user_data);
 void on_login_window_destroy(GtkWidget *widget, gpointer user_data);

static void on_login_button_clicked(GtkWidget *button, gpointer user_data) {
    CredentialForm *login_data = (CredentialForm *)user_data;
    const gchar *username = gtk_entry_get_text(login_data->entry_username);
    const gchar *password = gtk_entry_get_text(login_data->entry_password);

    if (g_strcmp0(username, "") == 0 || g_strcmp0(password, "") == 0) {
        show_notification_window(WARN, "Please fill in all required fields!");
        return;
    }

    User *user = createUser(NULL, main_session, username, password);
    if (user != NULL) {
        main_session->user = user;
        user->login(user);
    } else {
        g_on_show_notification("Failed to create user!");
    }
}



static void on_register_button_clicked(GtkWidget *button, gpointer user_data) {
    on_show_ui(MAIN_UI_LEVEL_REGISTER);
}

void on_login_window_destroy(GtkWidget *widget, gpointer data) {
    CredentialForm *login_data = (CredentialForm *)data;
    if (login_data) {
        
        g_free(login_data);
    }

}


void create_login_ui() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    CredentialForm *login_data = g_malloc(sizeof(CredentialForm));
    login_data->entry_username = GTK_ENTRY(gtk_entry_new());
    login_data->entry_password = GTK_ENTRY(gtk_entry_new());

    
    gtk_entry_set_visibility(login_data->entry_password, FALSE);

    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Username:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(login_data->entry_username), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Password:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(login_data->entry_password), FALSE, FALSE, 0);

    
    GtkWidget *btn_login = gtk_button_new_with_label("Login");
    g_signal_connect(btn_login, "clicked", G_CALLBACK(on_login_button_clicked), login_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_login, FALSE, FALSE, 0);

    
    GtkWidget *btn_register = gtk_button_new_with_label("Register new account");
    g_signal_connect(btn_register, "clicked", G_CALLBACK(on_register_button_clicked), login_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_register, FALSE, FALSE, 0);

    
    g_signal_connect(window, "destroy", G_CALLBACK(force_exit), login_data);

    gtk_widget_show_all(window);
    set_current_ui(window);
}


gboolean g_on_show_login_window(gpointer data) {
    create_login_ui();
    return false;
}
