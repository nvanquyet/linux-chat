#include <gtk/gtk.h>
#include "chat_common.h"  // Chứa khai báo: void show_main_window(void);

// Biến toàn cục cho cửa sổ tạo nhóm
GtkWidget *create_group_window = NULL;

// Callback xử lý khi nhấn nút "Create"
static void create_group_action(GtkWidget *widget, gpointer data) {
    // Ví dụ: Lấy thông tin từ các entry (nếu cần) và tạo nhóm
    // Hiện tại chỉ hiển thị hộp thoại thông báo thành công
    GtkWidget *message_dialog = gtk_message_dialog_new(
        GTK_WINDOW(create_group_window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Group created successfully!"
    );
    gtk_dialog_run(GTK_DIALOG(message_dialog));
    gtk_widget_destroy(message_dialog);
    
    // Hủy cửa sổ tạo nhóm
    gtk_widget_destroy(create_group_window);
    create_group_window = NULL;
    
    // Hiển thị lại cửa sổ chính (Chat Friend)
    show_main_window();
}

// Callback xử lý khi nhấn nút "Cancel" hoặc khi cửa sổ bị đóng
static void on_cancel_clicked(GtkWidget *widget, gpointer data) {
    gtk_widget_destroy(create_group_window);
    create_group_window = NULL;
    
    // Hiển thị lại cửa sổ chính
    show_main_window();
}

// Hàm tạo và hiển thị cửa sổ "Create Group"
void show_create_group_window(void) {
    GtkWidget *grid;
    GtkWidget *name_label, *search_label, *list_label;
    GtkWidget *name_entry, *search_entry, *list_entry;
    GtkWidget *create_button, *cancel_button;
    GtkWidget *button_box;
    
    // Tạo cửa sổ "Create Group"
    create_group_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(create_group_window), "Create Group");
    gtk_window_set_default_size(GTK_WINDOW(create_group_window), 400, 250);
    // Căn giữa cửa sổ trên màn hình
    gtk_window_set_position(GTK_WINDOW(create_group_window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(create_group_window), 20);
    // Nếu cửa sổ bị đóng theo cách nào đó, gọi on_cancel_clicked để xử lý
    g_signal_connect(create_group_window, "destroy", G_CALLBACK(on_cancel_clicked), NULL);
    
    // Tạo Grid để sắp xếp các widget
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_container_add(GTK_CONTAINER(create_group_window), grid);
    
    // Trường nhập tên nhóm
    name_label = gtk_label_new("Name Group:");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    
    name_entry = gtk_entry_new();
    gtk_widget_set_hexpand(name_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    
    // Trường nhập tìm kiếm bạn bè
    search_label = gtk_label_new("Search Friend:");
    gtk_label_set_xalign(GTK_LABEL(search_label), 0);
    gtk_grid_attach(GTK_GRID(grid), search_label, 0, 1, 1, 1);
    
    search_entry = gtk_entry_new();
    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), search_entry, 1, 1, 1, 1);
    
    // Trường nhập danh sách bạn được thêm vào nhóm
    list_label = gtk_label_new("List Add Friend:");
    gtk_label_set_xalign(GTK_LABEL(list_label), 0);
    gtk_grid_attach(GTK_GRID(grid), list_label, 0, 2, 1, 1);
    
    list_entry = gtk_entry_new();
    gtk_widget_set_hexpand(list_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), list_entry, 1, 2, 1, 1);
    
    // Hộp chứa các nút hành động
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), button_box, 0, 3, 2, 1);
    
    // Nút "Create"
    create_button = gtk_button_new_with_label("Create");
    g_signal_connect(create_button, "clicked", G_CALLBACK(create_group_action), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), create_button, FALSE, FALSE, 0);
    
    // Nút "Cancel"
    cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 0);
    
    // Hiển thị toàn bộ giao diện
    gtk_widget_show_all(create_group_window);
}
