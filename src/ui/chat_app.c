#include <gtk/gtk.h>
#include <pango/pango.h>
#include "chat_common.h"
#include "cmd.h"
#include "user.h"
#include "log.h"

// Biến toàn cục cho cửa sổ chính
GtkWidget *main_window = NULL;

// Forward declarations
void show_chat_window(Session *session);
void show_login_window(Session *session);

// Callback cho nút "Create Group"
static void on_create_group_clicked(GtkWidget *widget, gpointer data) {
    // Xử lý tạo nhóm
    // ...
}

// Callback cho nút "Log Out"
static void on_log_out_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;

    // Ẩn cửa sổ chính
    if (main_window != NULL) {
        gtk_widget_hide(main_window);
    }

    // Đặt trạng thái đăng nhập về FALSE
    session->isLogin = FALSE;

    // Gửi thông báo đăng xuất đến server
    Message msg;
    // msg.command = LOGOUT;
    // msg.sender = session->user;
    // send_message(session->conn_sock, &msg);

    // Hiển thị lại cửa sổ đăng nhập
    show_login_window(session);
}

// Callback khi cửa sổ chính bị đóng
static void on_main_window_destroy(GtkWidget *widget, gpointer data) {
    // Thoát toàn bộ ứng dụng
    gtk_main_quit();
}

// Callback an toàn để hiển thị cửa sổ chat từ thread chính GTK
gboolean show_chat_window_callback(gpointer data) {
    Session *session = (Session *)data;
    show_chat_window(session);
    return G_SOURCE_REMOVE; // G_SOURCE_REMOVE để chỉ thực hiện một lần
}

// Callback hiển thị thông báo lỗi đăng nhập từ thread chính GTK
gboolean show_login_error_callback(gpointer data) {
    char *error_message = (char *)data;

    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                        GTK_DIALOG_MODAL,
                        GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_OK,
                        "Đăng nhập thất bại: %s", error_message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    g_free(error_message); // Giải phóng bộ nhớ cho thông báo lỗi
    return G_SOURCE_REMOVE;
}

// Hàm xử lý tin nhắn nhận được từ server (được gọi từ luồng collector)
void on_server_message(Session *session, Message *msg) {
    // Xử lý các loại tin nhắn nhận được từ server
    log_message(INFO, "Nhận tin nhắn từ server: command=%d", msg->command);

    switch (msg->command) {
        case LOGIN_SUCCESS:
            log_message(INFO, "Đăng nhập thành công cho user: %s", session->user->username);
            session->isLogin = TRUE;
            // Sử dụng g_idle_add để đảm bảo hiển thị cửa sổ chat trong luồng chính GTK
            g_idle_add((GSourceFunc)show_chat_window_callback, session);
            break;

//         case LOGIN_FAILED:
//             // Tạo bản sao của thông báo lỗi để hiển thị trong thread chính
//             char *error_message = g_strdup(msg->content);
//             log_message(ERROR, "Đăng nhập thất bại: %s", error_message);
//             // Hiển thị thông báo lỗi đăng nhập trong thread chính
//             g_idle_add((GSourceFunc)show_login_error_callback, error_message);
//             break;
//
//         case CHAT_MESSAGE:
//             // Xử lý tin nhắn chat đến
//             if (main_window != NULL && gtk_widget_get_visible(main_window)) {
//                 log_message(INFO, "Nhận tin nhắn chat từ %s: %s", msg->sender->username, msg->content);
//
//                 // Tìm text view và thêm tin nhắn mới vào (sử dụng g_idle_add để chạy trong thread chính GTK)
//                 gchar *formatted_msg = g_strdup_printf("%s: %s\n", msg->sender->username, msg->content);
//                 g_idle_add((GSourceFunc)add_message_to_chat, formatted_msg);
//             }
//             break;
//
//         case FRIEND_LIST:
//             // Cập nhật danh sách bạn bè (sử dụng g_idle_add để chạy trong thread chính GTK)
//             log_message(INFO, "Nhận danh sách bạn bè");
//             g_idle_add((GSourceFunc)update_friend_list, g_strdup(msg->content));
//             break;
//
//         // Xử lý các loại tin nhắn khác
//         case REGISTER_SUCCESS:
//             log_message(INFO, "Đăng ký thành công");
//             // Thông báo đăng ký thành công và chuyển về màn hình đăng nhập
//             g_idle_add((GSourceFunc)show_registration_success, session);
//             break;
//
//         case REGISTER_FAILED:
//             log_message(ERROR, "Đăng ký thất bại: %s", msg->content);
//             // Hiển thị thông báo lỗi đăng ký
//             g_idle_add((GSourceFunc)show_registration_error, g_strdup(msg->content));
//             break;
//
         default:
             log_message(INFO, "Nhận lệnh không xác định: %d", msg->command);
             break;
   }
}

// Hàm thêm tin nhắn vào khu vực chat (được gọi từ thread chính GTK)
gboolean add_message_to_chat(gpointer data) {
    gchar *formatted_msg = (gchar *)data;

    if (main_window != NULL) {
        GtkWidget *scrolled_window = gtk_grid_get_child_at(GTK_GRID(
            gtk_bin_get_child(GTK_BIN(main_window))), 1, 1);

        if (scrolled_window != NULL) {
            GtkWidget *chat_area = gtk_bin_get_child(GTK_BIN(scrolled_window));
            if (chat_area != NULL) {
                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_area));
                GtkTextIter end;
                gtk_text_buffer_get_end_iter(buffer, &end);

                gtk_text_buffer_insert(buffer, &end, formatted_msg, -1);

                // Cuộn xuống để hiển thị tin nhắn mới nhất
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chat_area),
                                             &end, 0.0, FALSE, 0.0, 0.0);
            }
        }
    }

    g_free(formatted_msg); // Giải phóng bộ nhớ
    return G_SOURCE_REMOVE;
}

// Hàm cập nhật danh sách bạn bè (được gọi từ thread chính GTK)
gboolean update_friend_list(gpointer data) {
    gchar *friend_list = (gchar *)data;

    if (main_window != NULL) {
        // Tìm contacts_box và cập nhật danh sách
        GtkWidget *left_box = gtk_grid_get_child_at(GTK_GRID(
            gtk_bin_get_child(GTK_BIN(main_window))), 0, 1);

        if (left_box != NULL) {
            // Giả sử contacts_box là widget thứ 3 trong left_box
            GList *children = gtk_container_get_children(GTK_CONTAINER(left_box));
            if (g_list_length(children) >= 3) {
                GtkWidget *contacts_box = g_list_nth_data(children, 2);

                // Xóa danh sách hiện tại
                GList *contact_children = gtk_container_get_children(GTK_CONTAINER(contacts_box));
                for (GList *l = contact_children; l != NULL; l = l->next) {
                    gtk_widget_destroy(GTK_WIDGET(l->data));
                }
                g_list_free(contact_children);

                // Thêm danh sách bạn bè mới từ friend_list
                // (Giả sử nội dung là danh sách tên bạn bè phân cách bởi dấu phẩy)
                gchar **friends = g_strsplit(friend_list, ",", -1);
                for (int i = 0; friends[i] != NULL; i++) {
                    GtkWidget *contact = gtk_button_new_with_label(g_strstrip(friends[i]));
                    gtk_widget_set_size_request(contact, -1, 40);
                   // g_signal_connect(contact, "clicked", G_CALLBACK(on_contact_clicked), NULL);
                    gtk_box_pack_start(GTK_BOX(contacts_box), contact, FALSE, FALSE, 0);
                    gtk_widget_show(contact);
                }
                g_strfreev(friends);
            }
            g_list_free(children);
        }
    }

    g_free(friend_list); // Giải phóng bộ nhớ
    return G_SOURCE_REMOVE;
}

// Hiển thị thông báo đăng ký thành công
gboolean show_registration_success(gpointer data) {
    Session *session = (Session *)data;

    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                        GTK_DIALOG_MODAL,
                        GTK_MESSAGE_INFO,
                        GTK_BUTTONS_OK,
                        "Đăng ký thành công! Vui lòng đăng nhập.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    // Chuyển về màn hình đăng nhập
    show_login_window(session);

    return G_SOURCE_REMOVE;
}

// Hiển thị thông báo lỗi đăng ký
gboolean show_registration_error(gpointer data) {
    gchar *error_message = (gchar *)data;

    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                        GTK_DIALOG_MODAL,
                        GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_OK,
                        "Đăng ký thất bại: %s", error_message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    g_free(error_message); // Giải phóng bộ nhớ
    return G_SOURCE_REMOVE;
}

// Hàm gửi tin nhắn chat
static void on_send_button_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;
    GtkWidget *message_box = gtk_grid_get_child_at(GTK_GRID(
        gtk_bin_get_child(GTK_BIN(main_window))), 0, 2);

    if (message_box != NULL) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(message_box));
        if (children != NULL) {
            GtkWidget *message_entry = GTK_WIDGET(children->data);
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(message_entry));

            // if (text != NULL && *text != '\0') {
            //     // Gửi tin nhắn đến server
            //     Message msg;
            //     msg.command = CHAT_MESSAGE;
            //     msg.sender = session->user;
            //
            //     // Sao chép nội dung tin nhắn
            //     strncpy(msg.content, text, MAX_CONTENT_LENGTH - 1);
            //     msg.content[MAX_CONTENT_LENGTH - 1] = '\0';
            //
            //     // Lấy tên người nhận từ contact được chọn (nếu có)
            //     if (session->selected_contact != NULL) {
            //         msg.receiver = session->selected_contact;
            //     }
            //
            //     // Gửi tin nhắn
            //     log_message(INFO, "Gửi tin nhắn: %s", text);
            //     send_message(session->conn_sock, &msg);
            //
            //     // Xóa nội dung đã nhập
            //     gtk_entry_set_text(GTK_ENTRY(message_entry), "");
            //
            //     // Hiển thị tin nhắn vừa gửi trong chat area
            //     gchar *formatted_msg = g_strdup_printf("You: %s\n", text);
            //     add_message_to_chat(formatted_msg);
            // }
            // g_list_free(children);
        }
    }
}

// Hàm chọn contact
static void on_contact_clicked(GtkWidget *widget, gpointer data) {
    Session *session = (Session *)data;
    const gchar *contact_name = gtk_button_get_label(GTK_BUTTON(widget));

    // Lưu contact đã chọn vào session
    // if (session->selected_contact != NULL) {
    //     g_free(session->selected_contact);
    // }
    // session->selected_contact = g_strdup(contact_name);

    // Cập nhật tiêu đề cửa sổ chat
    gchar *title = g_strdup_printf("Chat with %s", contact_name);
    gtk_window_set_title(GTK_WINDOW(main_window), title);
    g_free(title);

    log_message(INFO, "Đã chọn contact: %s", contact_name);

    // Yêu cầu lịch sử chat với contact này từ server
    Message msg;
    // msg.command = REQUEST_CHAT_HISTORY;
    // msg.sender = session->user;
    //
    // // Đặt người nhận là contact đã chọn
    // msg.receiver = session->selected_contact;
    //
    // // Gửi yêu cầu lịch sử chat
    // send_message(session->conn_sock, &msg);
}

// Hàm tạo và hiển thị cửa sổ chính (Chat Friend)
void show_chat_window(Session *session) {
    // Nếu cửa sổ đã được tạo trước đó, chỉ cần hiển thị lại
    if (main_window != NULL) {
        gtk_widget_show_all(main_window);
        return;
    }

    log_message(INFO, "Tạo cửa sổ chat chính");

    // Khai báo các widget cần thiết
    GtkWidget *grid;
    GtkWidget *title_label;
    GtkWidget *search_label, *search_entry;
    GtkWidget *contacts_box;
    GtkWidget *scrolled_window;
    GtkWidget *chat_area;
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
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
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

        // Gửi yêu cầu lấy danh sách bạn bè từ server
        Message msg;
        // msg.command = REQUEST_FRIEND_LIST;
        // msg.sender = session->user;
        //
        // log_message(INFO, "Gửi yêu cầu danh sách bạn bè");
        // send_message(session->conn_sock, &msg);

        // Trước khi nhận được danh sách từ server, hiển thị thông báo đang tải
        GtkWidget *loading_label = gtk_label_new("Đang tải danh sách bạn bè...");
        gtk_box_pack_start(GTK_BOX(contacts_box), loading_label, FALSE, FALSE, 0);
    }

    // -- Cột 1: Khu vực chat (scrolled) --
    {
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
    }

    // -- Cột 2: Hộp chứa các nút chức năng --
    {
        right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_grid_attach(GTK_GRID(grid), right_box, 2, 1, 1, 1);

        create_group_button = gtk_button_new_with_label("Create Group");
        g_signal_connect(create_group_button, "clicked", G_CALLBACK(on_create_group_clicked), session);
        gtk_box_pack_start(GTK_BOX(right_box), create_group_button, FALSE, FALSE, 0);

        add_friend_button = gtk_button_new_with_label("Add Friend");
        gtk_box_pack_start(GTK_BOX(right_box), add_friend_button, FALSE, FALSE, 0);

        logout_button = gtk_button_new_with_label("Log Out");
        g_signal_connect(logout_button, "clicked", G_CALLBACK(on_log_out_clicked), session);
        gtk_box_pack_start(GTK_BOX(right_box), logout_button, FALSE, FALSE, 0);
    }

    // === Hàng 2: Thanh nhập tin nhắn ===
    {
        message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_grid_attach(GTK_GRID(grid), message_box, 0, 2, 3, 1);

        message_entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(message_box), message_entry, TRUE, TRUE, 0);

        add_file_button = gtk_button_new_with_label("Add File");
        gtk_box_pack_start(GTK_BOX(message_box), add_file_button, FALSE, FALSE, 0);

        send_button = gtk_button_new_with_label("Send");
        g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), session);
        gtk_box_pack_start(GTK_BOX(message_box), send_button, FALSE, FALSE, 0);

        // Cho phép gửi tin nhắn bằng phím Enter
        g_signal_connect(message_entry, "activate", G_CALLBACK(on_send_button_clicked), session);
    }

    // Hiển thị toàn bộ giao diện
    gtk_widget_show_all(main_window);
}