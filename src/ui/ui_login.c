#include <gtk/gtk.h>
#include <session.h>
#include <ui_controller.h>

#include "log.h"
#include "user.h"


// Cấu trúc dữ liệu cho giao diện đăng nhập

// Prototype các hàm
static void show_message_dialog(GtkWindow *parent, const gchar *message, gboolean success);
static void on_login_button_clicked(GtkWidget *button, gpointer user_data);
static void on_register_button_clicked(GtkWidget *button, gpointer user_data);
 void on_login_window_destroy(GtkWidget *widget, gpointer user_data);
gboolean show_login_window_callback(gpointer data);

// Hiển thị hộp thoại thông báo
static void show_message_dialog(GtkWindow *parent, const gchar *message, gboolean success) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        success ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message
    );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Xử lý nút "Đăng nhập"
static void on_login_button_clicked(GtkWidget *button, gpointer user_data) {
    LoginData *login_data = (LoginData *)user_data;
    const gchar *username = gtk_entry_get_text(login_data->entry_username);
    const gchar *password = gtk_entry_get_text(login_data->entry_password);
    GtkWindow *parent_window = GTK_WINDOW(login_data->window);
    Session *session = login_data->session;

    if (g_strcmp0(username, "") == 0 || g_strcmp0(password, "") == 0) {
        show_message_dialog(parent_window, "Vui lòng nhập đầy đủ thông tin!", FALSE);
        return;
    }

    // Kiểm tra kết nối
    if (session == NULL || !session->connected) {
        show_message_dialog(parent_window, "Lỗi kết nối máy chủ!", FALSE);
        return;
    }

    User *user = createUser(NULL, session, username, password);
    if (user != NULL) {
        session->user = user;
        user->login(user);

    } else {
        show_message_dialog(parent_window, "Lỗi tạo người dùng!", FALSE);
    }
}

// Xử lý nút "Đăng ký"
static void on_register_button_clicked(GtkWidget *button, gpointer user_data) {
    on_show_ui(REGISTER);
}

// Giải phóng bộ nhớ khi đóng cửa sổ
// Hàm callback khi cửa sổ bị đóng
void on_login_window_destroy(GtkWidget *widget, gpointer data) {
    if (login_data) {
        // Giải phóng data cấp phát bằng g_malloc
        g_free(login_data);
    }

    log_message(INFO, "Login window destroyed and memory cleaned");
}


// Tạo và hiển thị cửa sổ đăng nhập
void *show_login_window(Session *session) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Đăng nhập");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    LoginData *login_data = g_malloc(sizeof(LoginData));
    login_data->window = window;
    login_data->entry_username = GTK_ENTRY(gtk_entry_new());
    login_data->entry_password = GTK_ENTRY(gtk_entry_new());
    login_data->session = session;  // Lưu lại session được truyền vào

    // Ẩn mật khẩu
    gtk_entry_set_visibility(login_data->entry_password, FALSE);

    // Layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Username:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(login_data->entry_username), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Password:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(login_data->entry_password), FALSE, FALSE, 0);

    // Nút Login
    GtkWidget *btn_login = gtk_button_new_with_label("Login");
    g_signal_connect(btn_login, "clicked", G_CALLBACK(on_login_button_clicked), login_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_login, FALSE, FALSE, 0);

    // Nút Register
    GtkWidget *btn_register = gtk_button_new_with_label("Đăng ký tài khoản mới");
    g_signal_connect(btn_register, "clicked", G_CALLBACK(on_register_button_clicked), login_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_register, FALSE, FALSE, 0);
    // Đóng cửa sổ
    g_signal_connect(window, "destroy", G_CALLBACK(on_login_window_destroy), login_data);

    gtk_widget_show_all(window);

}
