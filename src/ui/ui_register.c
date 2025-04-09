#include <gtk/gtk.h>
#include <glib.h>
#include <session.h>
#include <ui_controller.h>

#include "log.h"
#include "user.h"
extern Session* main_session;
extern GtkWidget* current_ui;
// Prototype các hàm
static void show_message_dialog(GtkWindow *parent, const gchar *message, gboolean success);
static void on_register_button_clicked(GtkWidget *button, gpointer user_data);
static void on_back_button_clicked(GtkWidget *button, gpointer user_data);
static void on_registration_window_destroy(GtkWidget *widget, gpointer user_data);
gboolean g_on_show_register_window(gpointer data);
void show_register_window();

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

// Xử lý nút "Đăng ký"
static void on_register_button_clicked(GtkWidget *button, gpointer user_data) {
    CredentialForm *reg_data = (CredentialForm *)user_data;
    const gchar *username = gtk_entry_get_text(reg_data->entry_username);
    const gchar *password = gtk_entry_get_text(reg_data->entry_password);
    const gchar *confirm  = gtk_entry_get_text(reg_data->entry_confirm);


    if (g_strcmp0(username, "") == 0 || g_strcmp0(password, "") == 0) {
        //show_message_dialog(parent_window, "Vui lòng nhập đầy đủ thông tin!", FALSE);
        return;
    }

    if (g_strcmp0(password, confirm) != 0) {
      //  show_message_dialog(parent_window, "Mật khẩu xác nhận không khớp!", FALSE);
        return;
    }

    // Đảm bảo session không phải NULL trước khi sử dụng

    // Kiểm tra kết nối
    log_message(INFO, "SESSION IS %s", main_session->connected ? "connected" : "disconnected");

    User *user = createUser(NULL, main_session, username, password);
    if (user != NULL) {
        main_session->user = user;
        user->userRegister(user);
       // show_message_dialog(parent_window, "Đã gửi yêu cầu đăng ký!", TRUE);
      //  gtk_widget_hide(reg_data->window);
        on_show_ui(MAIN_UI_LEVEL_LOGIN);  // Truyền lại session
    } else {
        //show_message_dialog(parent_window, "Lỗi tạo người dùng!", FALSE);
    }
}

// Xử lý nút "Quay lại"
static void on_back_button_clicked(GtkWidget *button, gpointer user_data) {
    CredentialForm *reg_data = (CredentialForm *)user_data;
    //Session *session = reg_data->session;  // Lấy session

   // gtk_widget_hide(reg_data->window);
    show_login_window();  // Truyền session khi quay lại login
}

// Giải phóng bộ nhớ khi đóng cửa sổ
static void on_registration_window_destroy(GtkWidget *widget, gpointer user_data) {
    CredentialForm *reg_data = (CredentialForm *)user_data;
    g_free(reg_data);
}

// Tạo và hiển thị cửa sổ đăng ký
void create_registration_ui() {

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    current_ui = GTK_WIDGET(window);
    gtk_window_set_title(GTK_WINDOW(window), "Đăng ký");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 250);

    CredentialForm *reg_data = g_malloc(sizeof(CredentialForm));
   // reg_data->window = window;
    reg_data->entry_username = GTK_ENTRY(gtk_entry_new());
    reg_data->entry_password = GTK_ENTRY(gtk_entry_new());
    reg_data->entry_confirm = GTK_ENTRY(gtk_entry_new());


    // Ẩn mật khẩu
    gtk_entry_set_visibility(reg_data->entry_password, FALSE);
    gtk_entry_set_visibility(reg_data->entry_confirm, FALSE);

    // Layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Username:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(reg_data->entry_username), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Password:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(reg_data->entry_password), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Confirm Password:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(reg_data->entry_confirm), FALSE, FALSE, 0);

    // Nút Register
    GtkWidget *btn_register = gtk_button_new_with_label("Register");
    g_signal_connect(btn_register, "clicked", G_CALLBACK(on_register_button_clicked), reg_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_register, FALSE, FALSE, 0);

    // Nút Quay lại
    GtkWidget *btn_back = gtk_button_new_with_label("Quay lại đăng nhập");
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_button_clicked), reg_data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_back, FALSE, FALSE, 0);

    // Đóng cửa sổ
    g_signal_connect(window, "destroy", G_CALLBACK(on_registration_window_destroy), reg_data);

    gtk_widget_show_all(window);
}
gboolean g_on_show_register_window(gpointer data) {
    create_registration_ui();
    return false;
}