#include <gtk/gtk.h>
#include <pango/pango.h>
#include "chat_common.h"

// Biến toàn cục cho cửa sổ chính
GtkWidget *main_window = NULL;

// Callback cho nút "Create Group"
static void on_create_group_clicked(GtkWidget *widget, gpointer data) {
    // Ẩn cửa sổ chính
    gtk_widget_hide(main_window);
    // Hiển thị cửa sổ tạo nhóm (hàm này được định nghĩa trong create_group.c)
    show_create_group_window();
}

// Callback cho nút "Log Out"
static void on_log_out_clicked(GtkWidget *widget, gpointer data) {
    // Ẩn cửa sổ chính
    gtk_widget_hide(main_window);
    show_login_window();  // Gọi hàm hiển thị cửa sổ đăng nhập
    // show_login_window();
    // Hoặc bạn có thể thoát ứng dụng:
    // gtk_main_quit();
}

// Callback khi cửa sổ chính bị đóng
static void on_main_window_destroy(GtkWidget *widget, gpointer data) {
    // Thoát toàn bộ ứng dụng
    gtk_main_quit();
}

// Hàm tạo và hiển thị cửa sổ chính (Chat Friend)
void show_main_window(void) {
    // Nếu cửa sổ đã được tạo trước đó, chỉ cần hiển thị lại
    if (main_window != NULL) {
        gtk_widget_show_all(main_window);
        return;
    }
    
    // Khai báo các widget cần thiết
    GtkWidget *grid;
    GtkWidget *title_label;
    GtkWidget *search_label, *search_entry;
    GtkWidget *contacts_box;
    GtkWidget *contact;
    GtkWidget *chat_area;
    GtkWidget *scrolled_window;
    GtkWidget *message_box;
    GtkWidget *message_entry;
    GtkWidget *add_file_button;
    GtkWidget *send_button;
    GtkWidget *right_box;
    GtkWidget *create_group_button;
    GtkWidget *add_friend_button;
    GtkWidget *logout_button;

    // Tạo cửa sổ chính
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Chat Friend");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1000, 600);
    // Căn giữa cửa sổ trên màn hình
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    // Khi đóng cửa sổ thì thoát chương trình
    g_signal_connect(main_window, "destroy", G_CALLBACK(on_main_window_destroy), NULL);

    // Tạo layout chính dùng Grid (3 cột, 3 hàng)
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(main_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);

    // === Hàng 0: Tiêu đề lớn "Chat Friend" (span 3 cột) ===
    title_label = gtk_label_new("Chat Friend");
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    // Dùng Pango để font to, đậm
    {
        PangoAttrList *attr_list = pango_attr_list_new();
        PangoAttribute *attr_weight = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
        pango_attr_list_insert(attr_list, attr_weight);
        PangoAttribute *attr_scale = pango_attr_scale_new(2.0);
        pango_attr_list_insert(attr_list, attr_scale);
        gtk_label_set_attributes(GTK_LABEL(title_label), attr_list);
        pango_attr_list_unref(attr_list);
    }
    // Đặt vào grid, chiếm 3 cột (col=0..2)
    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 3, 1);

    // === Hàng 1: Cột 0 là Search + Contacts, Cột 1 là Chat, Cột 2 là Right box ===

    // -- Cột 0: Tìm kiếm và danh bạ --
    {
        // Tạo box dọc chứa Search + Contacts
        GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_grid_attach(GTK_GRID(grid), left_box, 0, 1, 1, 1);

        // Nhãn Search
        search_label = gtk_label_new("Search");
        gtk_widget_set_halign(search_label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(left_box), search_label, FALSE, FALSE, 0);

        // Ô nhập Search
        search_entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(left_box), search_entry, FALSE, FALSE, 0);

        // Box chứa danh sách contact
        contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_widget_set_size_request(contacts_box, 200, 300);
        gtk_box_pack_start(GTK_BOX(left_box), contacts_box, TRUE, TRUE, 0);

        // Thêm một số danh bạ mẫu
        for (int i = 0; i < 6; i++) {
            contact = gtk_button_new_with_label("Contact");
            gtk_widget_set_size_request(contact, -1, 40);
            gtk_box_pack_start(GTK_BOX(contacts_box), contact, FALSE, FALSE, 0);
        }
    }

    // -- Cột 1: Khu vực chat (scrolled) --
    {
        scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_widget_set_size_request(scrolled_window, 400, 300);

        // Tạo text view
        chat_area = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_area), FALSE);
        // Cho text view vào scrolled
        gtk_container_add(GTK_CONTAINER(scrolled_window), chat_area);

        // Đặt scrolled_window vào grid (col=1, row=1)
        gtk_grid_attach(GTK_GRID(grid), scrolled_window, 1, 1, 1, 1);

        // Cho phép scrolled_window mở rộng theo chiều dọc
        gtk_widget_set_vexpand(scrolled_window, TRUE);
    }

    // -- Cột 2: Hộp chứa các nút chức năng (Create Group, Add Friend, Log Out) --
    {
        right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_grid_attach(GTK_GRID(grid), right_box, 2, 1, 1, 1);

        create_group_button = gtk_button_new_with_label("Create Group");
        g_signal_connect(create_group_button, "clicked", G_CALLBACK(on_create_group_clicked), NULL);
        gtk_box_pack_start(GTK_BOX(right_box), create_group_button, FALSE, FALSE, 0);

        add_friend_button = gtk_button_new_with_label("Add Friend");
        gtk_box_pack_start(GTK_BOX(right_box), add_friend_button, FALSE, FALSE, 0);

        // Thêm nút "Log Out"
        logout_button = gtk_button_new_with_label("Log Out");
        g_signal_connect(logout_button, "clicked", G_CALLBACK(on_log_out_clicked), NULL);
        gtk_box_pack_start(GTK_BOX(right_box), logout_button, FALSE, FALSE, 0);
    }

    // === Hàng 2: Thanh nhập tin nhắn (span 2 cột: col=1..2) ===
    {
        message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        // Đặt vào grid tại (col=0..2)? Ở đây ta chỉ cần col=0..2 nếu muốn spanning
        // Nhưng ta chỉ muốn chat area + right box chung 2 cột, nên ta có 3 cột
        // Thường logic: ta đặt message_box ngay bên dưới chat area (col=1) và spanning 2 cột (1..2).
        gtk_grid_attach(GTK_GRID(grid), message_box, 0, 2, 3, 1);

        message_entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(message_box), message_entry, TRUE, TRUE, 0);

        add_file_button = gtk_button_new_with_label("Add File");
        gtk_box_pack_start(GTK_BOX(message_box), add_file_button, FALSE, FALSE, 0);

        send_button = gtk_button_new_with_label("Send");
        gtk_box_pack_start(GTK_BOX(message_box), send_button, FALSE, FALSE, 0);
    }

    // Hiển thị toàn bộ giao diện
    gtk_widget_show_all(main_window);
}
