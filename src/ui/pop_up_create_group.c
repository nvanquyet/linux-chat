#include <log.h>
#include <stdbool.h>
#include <ui_controller.h>
#include <gtk/gtk.h>



GtkWidget *create_group_window = NULL;
gboolean g_on_show_create_window(gpointer data) ;
void show_create_group_window();
// Hàm xử lý khi nhấn nút "Create"
void create_group_action(GtkWidget *widget, gpointer data) {
    CredentialForm *cg_data = (CredentialForm *)data;
    GtkWidget *name_entry = cg_data->entry_username;
    GtkWidget *pass_enry = cg_data->entry_password;

    const gchar *group_name = gtk_entry_get_text(GTK_ENTRY(name_entry));
    if (!group_name || g_strcmp0(group_name, "") == 0) {
        log_message(WARN, "Group name is empty");
        return;
    }

    const gchar *group_password = gtk_entry_get_text(GTK_ENTRY(pass_enry));
    if (!group_password || g_strcmp0(group_password, "") == 0) {
        log_message(WARN, "Group password  is empty");
        return;
    }

    log_message(INFO, "Create group: %s %s", group_name, group_password);

    //send to server
    Service *self = main_session->service;
    self->create_group(self, main_session->user, group_name, group_password);

    if (create_group_window) {
        gtk_widget_destroy(create_group_window);
        create_group_window = NULL;
    }

    g_free(cg_data);
}


// Hàm tạo và hiển thị cửa sổ "Create Group"
void show_create_group_window() {
    GtkWidget *grid;
    GtkWidget *name_label, *pass_label;
    GtkWidget *name_entry, *pass_entry;
    GtkWidget *create_button, *cancel_button;
    GtkWidget *button_box;

    create_group_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    current_ui = create_group_window;
    current_ui = GTK_WIDGET(create_group_window);
    gtk_window_set_title(GTK_WINDOW(create_group_window), "Create Group");
    gtk_window_set_default_size(GTK_WINDOW(create_group_window), 400, 200);
    gtk_window_set_position(GTK_WINDOW(create_group_window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(create_group_window), 20);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_container_add(GTK_CONTAINER(create_group_window), grid);

    name_label = gtk_label_new("Name Group:");
    gtk_widget_set_halign(name_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);

    name_entry = gtk_entry_new();
    gtk_widget_set_hexpand(name_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);

    pass_label = gtk_label_new("Pass Group:");
    gtk_widget_set_halign(pass_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), pass_label, 0, 1, 1, 1);

    pass_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(pass_entry), FALSE);
    gtk_widget_set_hexpand(pass_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), pass_entry, 1, 1, 1, 1);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), button_box, 0, 2, 2, 1);

    create_button = gtk_button_new_with_label("Create");

    gtk_box_pack_start(GTK_BOX(button_box), create_button, FALSE, FALSE, 0);

    // (Tuỳ chọn) Thêm nút Cancel nếu muốn
    // cancel_button = gtk_button_new_with_label("Cancel");
    // gtk_box_pack_start(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 0);

    gtk_widget_show_all(create_group_window);
}
gboolean g_on_show_create_window(gpointer data) {
   show_create_group_window();
    return false;
}