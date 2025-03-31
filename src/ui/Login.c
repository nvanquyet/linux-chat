#include <gtk/gtk.h>
#include "chat_common.h"  // Chứa khai báo: void show_main_window(void); và void show_register_window(void); và void show_login_window(void);

// Callback xử lý khi nhấn vào link "Đăng ký ngay"
static void on_register_button_clicked(GtkWidget *button, gpointer user_data) {
    // Gọi giao diện đăng ký tích hợp trong cùng chương trình
    show_register_window();
    
    // Ẩn cửa sổ đăng nhập hiện tại
    gtk_widget_hide(GTK_WIDGET(user_data));
}

// Callback xử lý khi nhấn nút "Đăng nhập"
static void on_login_button_clicked(GtkWidget *button, gpointer user_data) {
    g_print("🟢 Đăng nhập thành công!\n");
    
    // Gọi giao diện Chat Friend (form chat_app)
    show_main_window();
    
    // Ẩn cửa sổ đăng nhập hiện tại
    gtk_widget_hide(GTK_WIDGET(user_data));
}

// Tạo giao diện đăng nhập
static GtkWidget* create_login_window(void) {
    GtkWidget *window;
    GtkWidget *main_vbox;       // Container chính dạng box dọc
    GtkWidget *label_title;     // Tiêu đề đăng nhập
    GtkWidget *grid;            // Grid chứa thông tin Tài khoản / Mật khẩu
    GtkWidget *label_user, *label_pass;
    GtkWidget *entry_user, *entry_pass;
    GtkWidget *btn_login;
    GtkWidget *btn_register;

    // Tạo cửa sổ đăng nhập
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Đăng nhập");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);

    // Tạo box dọc làm container chính
    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    // Tạo tiêu đề với font chữ to và đậm
    label_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_title),
                         "<span font='18' weight='bold'>ĐĂNG NHẬP</span>");
    gtk_widget_set_halign(label_title, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_vbox), label_title, FALSE, FALSE, 0);

    // Tạo lưới chứa ô nhập Tài khoản và Mật khẩu
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_vbox), grid, FALSE, FALSE, 0);

    // Nhãn và ô nhập Tài khoản
    label_user = gtk_label_new("Tài khoản:");
    entry_user = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), label_user, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_user, 1, 0, 1, 1);

    // Nhãn và ô nhập Mật khẩu
    label_pass = gtk_label_new("Mật khẩu:");
    entry_pass = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_pass), FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(entry_pass), '*');
    gtk_grid_attach(GTK_GRID(grid), label_pass, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_pass, 1, 1, 1, 1);

    // Nút Đăng nhập
    btn_login = gtk_button_new_with_label("Đăng nhập");
    gtk_widget_set_size_request(btn_login, 120, 40);
    gtk_widget_set_halign(btn_login, GTK_ALIGN_CENTER);
    g_signal_connect(btn_login, "clicked", G_CALLBACK(on_login_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(main_vbox), btn_login, FALSE, FALSE, 0);

    // Nút Đăng ký dạng link (sử dụng GtkLinkButton với URI "about:blank" để tránh lỗi URI rỗng)
    btn_register = gtk_link_button_new_with_label("about:blank", "Chưa có tài khoản? Đăng ký ngay");
    g_signal_connect(btn_register, "clicked", G_CALLBACK(on_register_button_clicked), window);
    gtk_widget_set_halign(btn_register, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_vbox), btn_register, FALSE, FALSE, 0);

    return window;
}

// Hàm hiển thị giao diện đăng nhập
void show_login_window(void) {
    GtkWidget *window = create_login_window();
    gtk_widget_show_all(window);
}


