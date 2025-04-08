#include <gtk/gtk.h>
#include <pango/pango.h>
#include "chat_common.h"
#include "cmd.h"
#include "user.h"
#include "log.h"

// Global variable for main window
GtkWidget *main_window = NULL;

// Handle contact selection
void on_contact_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;

    const gchar *type = g_object_get_data(G_OBJECT(widget), "type");
    const gchar *name = gtk_button_get_label(GTK_BUTTON(widget));

    if (g_strcmp0(type, "user") == 0) {
        int user_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "user_id"));
        log_message(INFO, "User selected: %s (ID: %d)", name, user_id);

        // Xử lý chat với user ở đây
        // session->selected_user_id = user_id;

    } else if (g_strcmp0(type, "group") == 0) {
        log_message(INFO, "Group selected: %s", name);

        // Xử lý chat nhóm ở đây
        // session->selected_group_name = name; (nếu có)

    } else {
        log_message(WARN, "Unknown contact type clicked: %s", type);
    }
}


//hande btn join Group
 void on_join_group_clicked(GtkWidget *widget, gpointer data) {
    ChatApp *chat_app = (ChatApp *)data;
    Session *session = chat_app->session;
    if (session == NULL) {
        log_message(ERROR, "Session pointer is NULL!");
        return;
    }

    log_message(INFO, "Create Group button clicked");
    // Gọi hàm hiển thị form tạo nhóm
    show_join_group_window(session);
}
// Update friend list in the UI
gboolean update_friend_list(gpointer data) {
    FriendListData *fl_data = (FriendListData *)data;
    gchar *friend_list = fl_data->friend_list;
    Session *session = fl_data->session;

    // Check if main_window is initialized
    if (main_window == NULL) {
        g_print("Error: main_window not initialized\n");
        g_free(friend_list);
        g_free(fl_data);
        return G_SOURCE_REMOVE;
    }

    // Get grid from main_window
    GtkWidget *grid = gtk_bin_get_child(GTK_BIN(main_window));
    if (!GTK_IS_GRID(grid)) {
        g_print("Error: Grid not found in main_window\n");
        g_free(friend_list);
        g_free(fl_data);
        return G_SOURCE_REMOVE;
    }

    // Get left_box from grid
    GtkWidget *left_box = gtk_grid_get_child_at(GTK_GRID(grid), 0, 1);
    if (left_box == NULL) {
        g_print("Error: left_box not found in grid\n");
        g_free(friend_list);
        g_free(fl_data);
        return G_SOURCE_REMOVE;
    }

    // Get children of left_box
    GList *children = gtk_container_get_children(GTK_CONTAINER(left_box));
    if (g_list_length(children) < 3) {
        g_print("Error: left_box doesn't have enough elements\n");
        g_list_free(children);
        g_free(friend_list);
        g_free(fl_data);
        return G_SOURCE_REMOVE;
    }

    // Get contacts_box (third element)
    GtkWidget *contacts_box = g_list_nth_data(children, 2);
    if (!GTK_IS_BOX(contacts_box)) {
        g_print("Error: contacts_box is not a GtkBox\n");
        g_list_free(children);
        g_free(friend_list);
        g_free(fl_data);
        return G_SOURCE_REMOVE;
    }

    // Remove existing widgets from contacts_box
    gtk_container_foreach(GTK_CONTAINER(contacts_box), (GtkCallback)gtk_widget_destroy, NULL);

    // Split friend list and add to contacts_box
    gchar **friends = g_strsplit(friend_list, ",", -1);
    for (int i = 0; friends[i] != NULL; i++) {
        gchar *entry = g_strstrip(friends[i]);
        gchar **parts = g_strsplit(entry, "/", 2);

        if (parts[0] && parts[1]) {
            gchar *username = parts[0];
            int user_id = atoi(parts[1]);

            // Tạo button với label là username
            GtkWidget *btn = gtk_button_new_with_label(username);
            gtk_widget_set_size_request(btn, -1, 40);
            // Gắn user_id vào button để dùng sau khi click
            g_object_set_data(G_OBJECT(btn), "user_id", GINT_TO_POINTER(user_id));
            g_object_set_data(G_OBJECT(btn), "type", "user");
            // Gắn callback
            g_signal_connect(btn, "clicked", G_CALLBACK(on_contact_clicked), session);

            // Thêm vào box
            gtk_box_pack_start(GTK_BOX(contacts_box), btn, FALSE, FALSE, 0);
            gtk_widget_show(btn);
        }

        g_strfreev(parts);
    }
    // g_strfreev(entries);
    //
    // for (int i = 0; friends[i] != NULL; i++) {
    //     GtkWidget *contact = gtk_button_new_with_label(g_strstrip(friends[i]));
    //     gtk_widget_set_size_request(contact, -1, 40);
    //     g_signal_connect(contact, "clicked", G_CALLBACK(on_contact_clicked), session);
    //     gtk_box_pack_start(GTK_BOX(contacts_box), contact, FALSE, FALSE, 0);
    //     gtk_widget_show(contact);
    // }
    g_strfreev(friends);

    // Free memory
    g_list_free(children);
    g_free(friend_list);
    g_free(fl_data);
    return G_SOURCE_REMOVE;
}

// "Create Group" button callback
static void on_create_group_clicked(GtkWidget *widget, gpointer data) {
    ChatApp *chat_app = (ChatApp *)data;
    Session *session = chat_app->session;
    if (session == NULL) {
        log_message(ERROR, "Session pointer is NULL!");
        return;
    }

    log_message(INFO, "Create Group button clicked");
    // Gọi hàm hiển thị form tạo nhóm
    show_create_group_window(session);
}


// "Log Out" button callback
static void on_log_out_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;

    // Hide main window
    if (main_window != NULL) {
        gtk_widget_hide(main_window);
    }

    // Set login status to FALSE
    session->isLogin = FALSE;

    // Send logout notification to server
    Message *msg = message_create(LOGOUT);
    if (msg != NULL) {
        session_send_message(session, msg);
        log_message(INFO, "Sent LOGOUT request to server");
    }

    // Show login window
    show_login_window(session);
}

// Main window close callback
static void on_main_window_destroy(GtkWidget *widget, gpointer data) {
    // Exit application
    gtk_main_quit();
}

// Send button callback
static void on_send_button_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;

    if (main_window == NULL) {
        log_message(ERROR, "Main window is NULL");
        return;
    }

    GtkWidget *grid = gtk_bin_get_child(GTK_BIN(main_window));
    GtkWidget *message_box = gtk_grid_get_child_at(GTK_GRID(grid), 0, 2);

    if (message_box != NULL) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(message_box));
        if (children != NULL) {
            GtkWidget *message_entry = GTK_WIDGET(children->data);
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(message_entry));

            if (text != NULL && *text != '\0') {
                // Create message structure - implementation depends on your exact needs
                Message *msg = message_create(USER_MESSAGE);
                if (msg != NULL) {
                    // Set message content - implementation may need to be adjusted
                    message_write_string(msg, (char*)text);

                    // Send the message
                    session_send_message(session, msg);
                    log_message(INFO, "Sent message: %s", text);

                    // Clear input
                    gtk_entry_set_text(GTK_ENTRY(message_entry), "");

                    // Display sent message in chat area
                    gchar *formatted_msg = g_strdup_printf("You: %s\n", text);
                    g_idle_add(add_message_to_chat, formatted_msg);
                }
            }
            g_list_free(children);
        }
    }
}

// File send button callback
static void on_send_file_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;
    log_message(INFO, "Send File button clicked");

    // Implement file selection and sending process
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choose File to Send",
                                                    GTK_WINDOW(main_window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        log_message(INFO, "Selected file: %s", filename);

        // Send the file to server (add implementation based on your protocol)
        // Message *msg = message_create(SEND_FILE);
        // if (msg != NULL) {
        //     message_write_string(msg, filename);  // For example, sending filename first
        //     session_send_message(session, msg);
        //     log_message(INFO, "Sent file: %s", filename);
        // }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}


// Create and show main chat window
void show_chat_window(Session *session) {
    // If window already exists, just show it
    if (main_window != NULL) {
        gtk_widget_show_all(main_window);
        return;
    }

    log_message(INFO, "Creating main chat window");

    // Widget declarations
    GtkWidget *grid;
    GtkWidget *title_label;
    GtkWidget *search_label, *search_entry;
    GtkWidget *contacts_box;
    GtkWidget *scrolled_window;
    GtkWidget *chat_area;
    GtkWidget *message_box;
    GtkWidget *message_entry;
    GtkWidget *send_button;
    GtkWidget *send_file_button;
    GtkWidget *right_box;
    GtkWidget *create_group_button;
    GtkWidget *join_group_button;
    GtkWidget *logout_button;

    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Chat Friend");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    g_signal_connect(main_window, "destroy", G_CALLBACK(on_main_window_destroy), NULL);

    // Create main grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(main_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);

    // Main title
    title_label = gtk_label_new("Chat Friend");
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 3, 1);

    // Column 0: Search and contacts
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_grid_attach(GTK_GRID(grid), left_box, 0, 1, 1, 1);

    search_label = gtk_label_new("Search");
    gtk_widget_set_halign(search_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left_box), search_label, FALSE, FALSE, 0);

    search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(left_box), search_entry, FALSE, FALSE, 0);

    contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(contacts_box, 200, 300);
    gtk_box_pack_start(GTK_BOX(left_box), contacts_box, TRUE, TRUE, 0);

    // Show loading message before receiving list
    GtkWidget *loading_label = gtk_label_new("Loading friend list...");
    gtk_box_pack_start(GTK_BOX(contacts_box), loading_label, FALSE, FALSE, 0);

    // Request friend list from server
    Message *msg = message_create(GET_USERS);  // Tạo tin nhắn GET_USERS
    if (msg != NULL) {

        message_write_string(msg," ");
        session_send_message(session, msg);
        log_message(INFO, "Sent GET_USERS request to server");
    } else {
        log_message(ERROR, "Could not create GET_USERS message");
    }



    // Column 1: Chat area
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 400, 300);
    chat_area = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_area), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat_area), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scrolled_window), chat_area);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window, 1, 1, 1, 1);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);

    // Column 2: Function buttons
    right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_grid_attach(GTK_GRID(grid), right_box, 2, 1, 1, 1);

    create_group_button = gtk_button_new_with_label("Create Group");
    g_signal_connect(create_group_button, "clicked", G_CALLBACK(on_create_group_clicked), session);
    gtk_box_pack_start(GTK_BOX(right_box), create_group_button, FALSE, FALSE, 0);

    join_group_button = gtk_button_new_with_label("Join Group");
    g_signal_connect(join_group_button, "clicked", G_CALLBACK(on_join_group_clicked), session);
    gtk_box_pack_start(GTK_BOX(right_box), join_group_button, FALSE, FALSE, 0);

    logout_button = gtk_button_new_with_label("Log Out");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(on_log_out_clicked), session);
    gtk_box_pack_start(GTK_BOX(right_box), logout_button, FALSE, FALSE, 0);

    send_file_button = gtk_button_new_with_label("Send File");
    g_signal_connect(send_file_button, "clicked", G_CALLBACK(on_send_file_clicked), session);
    gtk_box_pack_start(GTK_BOX(right_box), send_file_button, FALSE, FALSE, 0);

    // Message input bar
    message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(grid), message_box, 0, 2, 3, 1);

    message_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(message_box), message_entry, TRUE, TRUE, 0);

    send_button = gtk_button_new_with_label("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), session);
    gtk_box_pack_start(GTK_BOX(message_box), send_button, FALSE, FALSE, 0);

    g_signal_connect(message_entry, "activate", G_CALLBACK(on_send_button_clicked), session);

    // Show all interface elements
    gtk_widget_show_all(main_window);
}

// Safe callback to display chat window from GTK main thread
gboolean show_chat_window_callback(gpointer data) {
    Session *session = (Session *)data;
    show_chat_window(session);
    return G_SOURCE_REMOVE; // Only execute once
}

// Add message to chat area (called from GTK main thread)
gboolean add_message_to_chat(gpointer data) {
    gchar *formatted_msg = (gchar *)data;

    if (main_window != NULL) {
        GtkWidget *scrolled_window = gtk_grid_get_child_at(GTK_GRID(
            gtk_bin_get_child(GTK_BIN(main_window))), 1, 1);

        if (scrolled_window != NULL) {
            GtkWidget *chat_area = gtk_bin_get_child(GTK_BIN(scrolled_window));
            if (GTK_IS_TEXT_VIEW(chat_area)) {
                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_area));
                GtkTextIter iter;
                gtk_text_buffer_get_end_iter(buffer, &iter);
                gtk_text_buffer_insert(buffer, &iter, formatted_msg, -1);
            }
        }
    }
    g_free(formatted_msg);
    return G_SOURCE_REMOVE;
}


