#include <log.h>
#include <gtk/gtk.h>
#include "chat_common.h"
GtkWidget *join_group_window = NULL;
void join_group_action(GtkAction *action, gpointer data) {
    JoinGroupData *jg_data = (JoinGroupData *)data;
    Session *session = jg_data->session;
    const gchar *group_name = gtk_entry_get_text(GTK_ENTRY(jg_data->name_entry));
    if (!group_name || g_strcmp0(group_name, "") == 0) {
        log_message(WARN, "Group name is empty");
        return;
    }

    const gchar *group_pass = gtk_entry_get_text(GTK_ENTRY(jg_data->pass_entry));
    if (!group_pass || g_strcmp0(group_pass, "") == 0) {
        log_message(WARN, "Group password  is empty");
        return;
    }

    log_message(INFO, "Join group: %s %s", group_name, group_pass);

    //send to server
    Service *self = session->service;
    self->join_group(self, session->user, group_name, group_pass);
    if (join_group_window) {
        gtk_widget_destroy(join_group_window);
        join_group_window = NULL;
    }

     g_free(jg_data);
}
void show_join_group_window(Session *session) {
    GtkWidget *grid;
    GtkWidget *group_name_label, *group_pass_label;
    GtkWidget *group_name_entry, *group_pass_entry;
    GtkWidget *join_button, *cancel_button;
    GtkWidget *button_box;

    // Tạo cửa sổ
    join_group_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(join_group_window), "Join Group");
    gtk_window_set_default_size(GTK_WINDOW(join_group_window), 400, 200);
    gtk_window_set_position(GTK_WINDOW(join_group_window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(join_group_window), 20);

    // Tạo layout dạng lưới
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_container_add(GTK_CONTAINER(join_group_window), grid);

    // Label + Entry cho Group Name
    group_name_label = gtk_label_new("Group Name:");
    gtk_widget_set_halign(group_name_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), group_name_label, 0, 0, 1, 1);

    group_name_entry = gtk_entry_new();
    gtk_widget_set_hexpand(group_name_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), group_name_entry, 1, 0, 1, 1);

    // Label + Entry cho Group Password
    group_pass_label = gtk_label_new("Group Password:");
    gtk_widget_set_halign(group_pass_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), group_pass_label, 0, 1, 1, 1);

    group_pass_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(group_pass_entry), FALSE);
    gtk_widget_set_hexpand(group_pass_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), group_pass_entry, 1, 1, 1, 1);

    // Hộp chứa các nút
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), button_box, 0, 2, 2, 1);

    // Cấu trúc dữ liệu truyền qua callback
    JoinGroupData *jg_data = g_new0(JoinGroupData, 1);  // Bạn cần định nghĩa struct này
    jg_data->session = session;
    jg_data->name_entry = group_name_entry;
    jg_data->pass_entry = group_pass_entry;


    // Nút Join
    join_button = gtk_button_new_with_label("Join");
    g_signal_connect(join_button, "clicked", G_CALLBACK(join_group_action), jg_data);
    gtk_box_pack_start(GTK_BOX(button_box), join_button, FALSE, FALSE, 0);

    // (Tùy chọn) Nút Cancel
    cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), join_group_window);
    gtk_box_pack_start(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 0);

    gtk_widget_show_all(join_group_window);
}
