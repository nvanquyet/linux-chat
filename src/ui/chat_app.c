#include <gtk/gtk.h>

#include "chat_common.h"
#include "cmd.h"
#include "user.h"
#include "log.h"

// Global variable for main window
GtkWidget *main_window = NULL;
// Handle contact selection
// Add this global variable to track the selected user ID
int selected_user_id = -1;  // -1 means no user selected

// Modified contact selection handler
void on_contact_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;

    const gchar *type = g_object_get_data(G_OBJECT(widget), "type");
    const gchar *name = gtk_button_get_label(GTK_BUTTON(widget));

    if (g_strcmp0(type, "user") == 0) {
        // Store the selected user ID in the global variable
        selected_user_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "user_id"));
        log_message(INFO, "User selected: %s (ID: %d)", name, selected_user_id);

        // Update the chat header to indicate who you're chatting with
        if (main_window != NULL) {
            GtkWidget *grid = gtk_bin_get_child(GTK_BIN(main_window));
            GtkWidget *chat_header = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);

            if (chat_header == NULL) {
                // Create chat header if it doesn't exist
                chat_header = gtk_label_new("");
                gtk_grid_attach(GTK_GRID(grid), chat_header, 1, 0, 1, 1);
                gtk_widget_show(chat_header);
            }

            gchar *header_text = g_strdup_printf("Chatting with: %s", name);
            gtk_label_set_text(GTK_LABEL(chat_header), header_text);
            g_free(header_text);
        }

        Message *msg = message_create(GET_CHAT_HISTORY);
        if (msg != NULL) {
            char user_id_str[20];
            sprintf(user_id_str, "%d", selected_user_id);
            message_write_string(msg, user_id_str);
            session_send_message(session, msg);
            log_message(INFO, "Sent GET_CHAT_HISTORY request to server for user ID: %d", selected_user_id);
        } else {
            log_message(ERROR, "Could not create GET_CHAT_HISTORY message");
        }

    } else if (g_strcmp0(type, "group") == 0) {
        log_message(INFO, "Group selected: %s", name);
        // Reset the selected user ID since we're in a group
        selected_user_id = -1;

        // Group chat handling code here
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
gboolean add_message_to_chat(gpointer data) {
    gchar *formatted_msg = (gchar *)data;

    if (formatted_msg == NULL) {
        log_message(ERROR, "Received NULL message in add_message_to_chat");
        return G_SOURCE_REMOVE;
    }

    log_message(INFO, "Adding message to chat: '%s'", formatted_msg);

    if (main_window != NULL) {
        GtkWidget *grid = gtk_bin_get_child(GTK_BIN(main_window));
        if (grid == NULL) {
            log_message(ERROR, "Grid not found in main window");
            g_free(formatted_msg);
            return G_SOURCE_REMOVE;
        }

        GtkWidget *scrolled_window = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
        if (scrolled_window == NULL) {
            log_message(ERROR, "Scrolled window not found in grid");
            g_free(formatted_msg);
            return G_SOURCE_REMOVE;
        }

        GtkWidget *chat_area = gtk_bin_get_child(GTK_BIN(scrolled_window));
        if (!GTK_IS_TEXT_VIEW(chat_area)) {
            log_message(ERROR, "Chat area is not a GtkTextView");
            g_free(formatted_msg);
            return G_SOURCE_REMOVE;
        }

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_area));
        if (buffer == NULL) {
            log_message(ERROR, "Could not get text buffer from chat view");
            g_free(formatted_msg);
            return G_SOURCE_REMOVE;
        }

        // Insert at end of buffer
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, formatted_msg, -1);
        log_message(INFO, "Message inserted into chat buffer");

        // Scroll to the new position
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chat_area), &iter, 0.0, TRUE, 0.0, 1.0);
        log_message(INFO, "Scrolled to bottom of chat view");
    } else {
        log_message(ERROR, "Main window is NULL in add_message_to_chat");
    }

    g_free(formatted_msg);
    return G_SOURCE_REMOVE;
}
// Update friend list in the UI
// Update friend list in the UI
gboolean update_friend_list(gpointer data) {
    FriendListData *fl_data = (FriendListData *)data;
    gchar *friend_list = fl_data->friend_list;
    Session *session = fl_data->session;

    // Get current user ID for filtering
    int current_user_id = session->current_user_id;
    log_message(INFO, "Current user ID for filtering: %d", current_user_id);

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

            // Skip current user - Don't add them to the contact list
            if (user_id == current_user_id) {
                log_message(INFO, "Skipping current user in UI: %s (ID: %d)", username, user_id);
                g_strfreev(parts);
                continue;
            }

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

    // Hide the main window instead of destroying it
    if (main_window != NULL) {
        gtk_widget_hide(main_window);
        log_message(INFO, "Main window hidden for logout");
    }

    // Update logout status
    session->isLogin = FALSE;
    session->current_user_id = -1;

    // Send logout notification to server if user exists
    if (session->user != NULL) {
        session->user->logout(session->user);
        // Free user info if needed,
        // but make sure user->logout() handles sending notification without duplicate freeing.
        free(session->user);
        session->user = NULL;
    }

    // Don't need to add this here as the server response will trigger handle_logout
    // which will show the login window
}


// Main window close callback
static void on_main_window_destroy(GtkWidget *widget, gpointer data) {
    // Exit application
    gtk_main_quit();
}
// Send button callback
// Send button callback
static void on_send_button_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;

    if (main_window == NULL) {
        log_message(ERROR, "Main window is NULL");
        return;
    }

    // Kiểm tra xem đã chọn người dùng chưa
    if (selected_user_id == -1) {
        // Hiển thị thông báo cảnh báo
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_WARNING,
                                                  GTK_BUTTONS_OK,
                                                  "Vui lòng chọn liên hệ trước!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkWidget *grid = gtk_bin_get_child(GTK_BIN(main_window));
    if (grid == NULL) {
        log_message(ERROR, "Grid not found in main window");
        return;
    }

    // Lấy widget nhập tin nhắn
    GtkWidget *message_box = gtk_grid_get_child_at(GTK_GRID(grid), 0, 2);
    if (message_box == NULL) {
        log_message(ERROR, "Message box not found");
        return;
    }

    GList *children = gtk_container_get_children(GTK_CONTAINER(message_box));
    if (children == NULL) {
        log_message(ERROR, "Message entry not found");
        return;
    }

    GtkWidget *message_entry = GTK_WIDGET(children->data);
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(message_entry));

    // Debug nội dung tin nhắn
    log_message(INFO, "Message text content: '%s', length: %lu", text, (unsigned long)strlen(text));

    if (text != NULL && *text != '\0') {
        // Tạo một bản sao của text để giữ lại giá trị
        gchar *message_content = g_strdup(text);

        if (message_content != NULL) {
            // Tạo tin nhắn riêng tư với ID người dùng và nội dung tin nhắn
            Message *msg = message_create(USER_MESSAGE);
            if (msg != NULL) {
                // Viết ID người dùng đích trước
                char user_id_str[20];
                sprintf(user_id_str, "%d", selected_user_id);
                message_write_string(msg, user_id_str);
                log_message(INFO,"Current user id: %s", session->current_user_id);
                // Sau đó viết nội dung tin nhắn
                message_write_string(msg, message_content);
                session_send_message(session, msg);
                log_message(INFO, "Sent message to user ID %d: %s", selected_user_id, message_content);

                // Xóa trường nhập liệu
                gtk_entry_set_text(GTK_ENTRY(message_entry), "");

                // Định dạng tin nhắn để hiển thị
                log_message(INFO, "Formatting message with text: '%s'", message_content);
                gchar *formatted_msg = g_strdup_printf("You: %s\n", message_content);
                log_message(INFO, "Formatted message: '%s'", formatted_msg);

                // Hiển thị tin nhắn đã gửi trong khu vực chat
                GtkWidget *scrolled_window = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
                if (!GTK_IS_SCROLLED_WINDOW(scrolled_window)) {
                    log_message(ERROR, "Scrolled window not found or not a GtkScrolledWindow");
                    g_free(message_content);
                    g_list_free(children);
                    return;
                }

                GtkWidget *chat_view = gtk_bin_get_child(GTK_BIN(scrolled_window));
                if (!GTK_IS_TEXT_VIEW(chat_view)) {
                    log_message(ERROR, "Chat view is invalid or not a GtkTextView");
                    g_free(message_content);
                    g_list_free(children);
                    return;
                }

                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_view));
                if (buffer == NULL) {
                    log_message(ERROR, "Could not get text buffer from chat view");
                    g_free(message_content);
                    g_list_free(children);
                    return;
                }

                // Chèn ở cuối buffer
                GtkTextIter end;
                gtk_text_buffer_get_end_iter(buffer, &end);
                log_message(INFO, "End iterator position: %d", gtk_text_iter_get_offset(&end));

                // Chèn tin nhắn đã định dạng
                gtk_text_buffer_insert(buffer, &end, formatted_msg, -1);
                log_message(INFO, "Inserted message into buffer: %s", formatted_msg);
                g_free(formatted_msg);

                // Lấy vị trí cuối mới và cuộn đến đó
                gtk_text_buffer_get_end_iter(buffer, &end);
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chat_view), &end, 0.0, TRUE, 0.0, 1.0);
                log_message(INFO, "Scrolled to bottom of chat view");
            }
            g_free(message_content);
        }
    } else {
        log_message(WARN, "Attempted to send empty message");
    }

    g_list_free(children);
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




