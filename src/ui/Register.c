#include <gtk/gtk.h>
#include <glib.h>
#include "chat_common.h"  // Chứa khai báo: void show_login_window(void);

// Hàm hiển thị hộp thoại thông báo
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

// Callback cho nút "Quay lại đăng nhập"
// Khi nhấn, ẩn cửa sổ đăng ký hiện tại và gọi show_login_window() để hiển thị form đăng nhập
static void on_back_button_clicked(GtkWidget *button, gpointer user_data) {
    gtk_widget_hide(GTK_WIDGET(user_data));  // user_data ở đây là cửa sổ đăng ký
    show_login_window();
}

// Hàm xóa nội dung các ô nhập sau khi đăng ký thành công
static void clear_entries(GtkWidget **entries) {
    gtk_entry_set_text(GTK_ENTRY(entries[0]), ""); // Username
    gtk_entry_set_text(GTK_ENTRY(entries[1]), ""); // Password
    gtk_entry_set_text(GTK_ENTRY(entries[2]), ""); // Confirm Password
}

// Callback cho nút "Register"
static void on_register_button_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget **entries = (GtkWidget **)user_data;
    GtkWidget *entry_username = entries[0];
    GtkWidget *entry_password = entries[1];
    GtkWidget *entry_confirm  = entries[2];

    const gchar *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    const gchar *confirm  = gtk_entry_get_text(GTK_ENTRY(entry_confirm));

    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_toplevel(button));

    if (g_strcmp0(username, "") == 0 || g_strcmp0(password, "") == 0 || g_strcmp0(confirm, "") == 0) {
        show_message_dialog(parent_window, "Vui lòng nhập đầy đủ thông tin!", FALSE);
    } else if (g_strcmp0(password, confirm) != 0) {
        show_message_dialog(parent_window, "Mật khẩu xác nhận không khớp!", FALSE);
    } else {
        show_message_dialog(parent_window, "Đăng ký thành công!", TRUE);
        clear_entries(entries);
        // TODO: Thêm code lưu thông tin đăng ký vào cơ sở dữ liệu hoặc xử lý khác
    }
}

// Hàm tạo và hiển thị form đăng ký
void show_register_window(void) {
    GtkWidget *window, *vbox;
    GtkWidget *label_username, *label_password, *label_confirm;
    GtkWidget *entry_username, *entry_password, *entry_confirm;
    GtkWidget *btn_register, *btn_back;

    // Tạo cửa sổ đăng ký
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Đăng ký");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 250);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Tạo hộp dọc chứa các widget
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Nhãn và ô nhập Username
    label_username = gtk_label_new("Username:");
    gtk_box_pack_start(GTK_BOX(vbox), label_username, FALSE, FALSE, 0);
    entry_username = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_username, FALSE, FALSE, 0);

    // Nhãn và ô nhập Password
    label_password = gtk_label_new("Password:");
    gtk_box_pack_start(GTK_BOX(vbox), label_password, FALSE, FALSE, 0);
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), entry_password, FALSE, FALSE, 0);

    // Nhãn và ô nhập Confirm Password
    label_confirm = gtk_label_new("Confirm Password:");
    gtk_box_pack_start(GTK_BOX(vbox), label_confirm, FALSE, FALSE, 0);
    entry_confirm = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_confirm), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), entry_confirm, FALSE, FALSE, 0);

    // Nút "Register"
    btn_register = gtk_button_new_with_label("Register");
    {
        // Tạo mảng chứa các entry để truyền làm user_data cho callback
        GtkWidget *entries[3] = { entry_username, entry_password, entry_confirm };
        g_signal_connect(btn_register, "clicked", G_CALLBACK(on_register_button_clicked), entries);
    }
    gtk_box_pack_start(GTK_BOX(vbox), btn_register, FALSE, FALSE, 0);

    // Nút "Quay lại đăng nhập"
    btn_back = gtk_button_new_with_label("Quay lại đăng nhập");
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(vbox), btn_back, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
}
