#include <ui_controller.h>
#include <gtk/gtk.h>
#include "chat_message.h"
#include "cmd.h"
#include "user.h"
#include "log.h"

#define MAX_PREVIEW_LEN 20 // Số ký tự tối đa cho preview tin nhắn
/* Hàm trợ giúp cắt chuỗi nếu quá dài */

GtkWidget *home_widget = NULL;
extern Session* main_session;
extern GtkWidget* current_ui;
void update_or_create_contact(int id, const char *title, const char *message, long time, bool isGroup, bool bold);
static char* truncate_message(const char *msg) {
    if (strlen(msg) > MAX_PREVIEW_LEN) {
        char *short_msg = g_malloc(MAX_PREVIEW_LEN + 4); // Dự phòng thêm dấu "..."
        strncpy(short_msg, msg, MAX_PREVIEW_LEN);
        strcpy(short_msg + MAX_PREVIEW_LEN, "...");
        return short_msg;
    }
    return g_strdup(msg);
}
// Global variable for main window
int select_target_id = 0;

void on_join_group_clicked(GtkWidget *widget, gpointer data) {
    show_join_group_window();
}

// "Create Group" button callback
static void on_create_group_clicked(GtkWidget *widget, gpointer data) {
    show_create_group_window();
}

static void on_leave_group_clicked(GtkWidget *widget, gpointer data) {
    if (select_target_id >= 0)
    {
        show_notification_window(INFO, "Please select group to leave");
        return;
    }
    Service *self = main_session->service;
    self->leave_group(self, main_session->user, select_target_id);
}
static void on_delete_group_clicked(GtkWidget *widget, gpointer data) {
    if (select_target_id >= 0)
    {
        show_notification_window(INFO, "Please select group to leave");
        return;
    }
    Service *self = main_session->service;
    self->delete_group(self, main_session->user, select_target_id);
}

static void on_log_out_clicked(GtkWidget *widget, gpointer data) {
    // Don't need to add this here as the server response will trigger handle_logout
    // which will show the login window
    main_session->service->service_logout(main_session->user);
    on_show_ui(MAIN_UI_LEVEL_LOGIN);
}

// Main window close callback
static void on_main_window_destroy(GtkWidget *widget, gpointer data) {
    // Exit application
    gtk_main_quit();
}

//############----------Message------------
GtkWidget *chat_view = NULL;
static void insert_message_to_buffer(GtkTextBuffer *buffer, ChatMessage *msg) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    GtkTextTagTable *table = gtk_text_buffer_get_tag_table(buffer);

    GtkTextTag *tag = gtk_text_tag_table_lookup(table, msg->sender_id == main_session->user->id ? "self_user" : "other_user");
    if (!tag) {
        tag = gtk_text_tag_new(msg->sender_id == main_session->user->id ? "self_user" : "other_user");
        const char *color = (msg->sender_id == main_session->user->id) ? "gray" : "blue";
        g_object_set(tag, "foreground", color, NULL);
        gtk_text_tag_table_add(table, tag);
    }

    struct tm *timeinfo = gmtime(&msg->timestamp);
    char time_str[10] = "[??:??]";

    if (timeinfo) {
        strftime(time_str, sizeof(time_str), "[%H:%M]", timeinfo);
        char full_time_str[30];
        strftime(full_time_str, sizeof(full_time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
        log_message(INFO, "Timestamp converted to: %s", full_time_str);
    } else {
        log_message(WARN, "Invalid timestamp from message id %d", msg->sender_id);
    }
    char header[256];
    snprintf(header, sizeof(header), "%s %s: ", time_str,
             msg->sender_id == main_session->user->id ? "Me" : msg->sender_name);

    gtk_text_buffer_insert_with_tags(buffer, &iter, header, -1, tag, NULL);

    if (!msg->content || !g_utf8_validate(msg->content, -1, NULL)) {
        log_message(ERROR, "Invalid or NULL content from sender %d", msg->sender_id);
        gtk_text_buffer_insert(buffer, &iter, "<Invalid message>", -1);
    } else {
        gtk_text_buffer_insert(buffer, &iter, msg->content, -1);
    }

    gtk_text_buffer_insert(buffer, &iter, "\n", -1);

    GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(chat_view), mark);
}

void load_chat_history(ChatMessage *history, int count) {
    if (history == NULL || count <= 0) {
        log_message(WARN, "No chat history to load.");
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_view));
    gtk_text_buffer_set_text(buffer, "", -1);  // Clear nội dung cũ

    log_message(INFO, "Loading chat history: %d message(s)", count);

    for (int i = 0; i < count; ++i) {
        ChatMessage *msg = &history[i];

        if (msg == NULL || msg->content == NULL) {
            log_message(WARN, "Message at index %d is NULL or has NULL content", i);
            continue;
        }

        // Bọc thử-catch kiểu C (manual check)
        insert_message_to_buffer(buffer, msg);
    }

}


void append_chat_message(ChatMessage *msg) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_view));
    insert_message_to_buffer(buffer, msg);
}
void on_send_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *message_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "message_entry"));
    GtkWidget *chat_view = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "chat_view"));
    if (select_target_id != 0)
    {
        const char *content = gtk_entry_get_text(GTK_ENTRY(message_entry));
        if (!content || strlen(content) == 0) return;

        // Tạo ChatMessage mới
        ChatMessage *msg = g_new0(ChatMessage, 1);
        msg->sender_id = main_session->user->id;
        msg->sender_name = g_strdup("Me");
        msg->content = g_strdup(content);
        msg->timestamp = time(NULL) + 7 * 3600;

        // Thêm tin nhắn mới vào chat_view
        append_chat_message(msg);

        update_or_create_contact(select_target_id, "Unknown", g_strdup(content), time(NULL) + 7 * 3600, select_target_id < 0, false);
        if (select_target_id > 0)
        {
            //todo: send u-message
            main_session->service->send_user_message(main_session->service, main_session->user, select_target_id, msg->content);
        }
        if (select_target_id < 0)
        {
            //todo: send g-message
            main_session->service->send_group_message(main_session->service, main_session->user, -select_target_id, msg->content);
        }

        // Giải phóng nếu cần (ở đây không vì append_chat_message có thể cần giữ)
        g_free(msg->sender_name);
        g_free(msg->content);
        g_free(msg);

        // TODO: Gửi message tới server (nếu có)
    }


    // Xoá nội dung ô nhập
    gtk_entry_set_text(GTK_ENTRY(message_entry), "");
}
//######################------------Contact-----------####################
GtkWidget *contacts_box = NULL;
GHashTable *contact_map = NULL;
GtkWidget *contacts_scroll = NULL;

// Lấy hbox từ event_box (trong cấu trúc của chúng ta, event_box chứa một hbox)
static GtkWidget* get_hbox_from_event_box(GtkWidget *event_box) {
    return gtk_bin_get_child(GTK_BIN(event_box));
}

// Lấy vbox từ event_box (vbox là con đầu tiên của hbox)
static GtkWidget* get_vbox_from_event_box(GtkWidget *event_box) {
    GtkWidget *hbox = get_hbox_from_event_box(event_box);
    GList *children = gtk_container_get_children(GTK_CONTAINER(hbox));
    GtkWidget *vbox = g_list_nth_data(children, 0);
    g_list_free(children);
    return vbox;
}

// Lấy label từ vbox theo vị trí (index: 0 là title, 1 là message)
static GtkWidget* get_label_from_vbox(GtkWidget *vbox, int index) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(vbox));
    GtkWidget *label = g_list_nth_data(children, index);
    g_list_free(children);
    return label;
}

// Lấy time_label từ event_box (time_label được đặt ở vị trí 1 của hbox)
static GtkWidget* get_time_label_from_event_box(GtkWidget *event_box) {
    GtkWidget *hbox = get_hbox_from_event_box(event_box);
    GList *children = gtk_container_get_children(GTK_CONTAINER(hbox));
    GtkWidget *time_label = g_list_nth_data(children, 1);
    g_list_free(children);
    return time_label;
}
static gboolean is_click_disabled = FALSE;

gboolean enable_click(gpointer data) {
    is_click_disabled = FALSE;
    return G_SOURCE_REMOVE; // Chạy 1 lần rồi tự huỷ
}
gboolean on_contact_item_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (is_click_disabled)
    {
        return false;
    }
    is_click_disabled = TRUE;
    g_timeout_add(500, enable_click, NULL);
    GtkWidget *vbox = get_vbox_from_event_box(widget);
    GtkWidget *title_label = get_label_from_vbox(vbox, 0);
    GtkWidget *msg_label = get_label_from_vbox(vbox, 1);
    int target_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "id"));

    if (select_target_id == target_id) return FALSE;
    // Bỏ in đậm title nếu đang dùng markup
    // const char *plain_title = gtk_label_get_text(GTK_LABEL(title_label));
    // gtk_label_set_text(GTK_LABEL(title_label), plain_title);

    // Bỏ in đậm message nếu đang in đậm
    const char *plain_msg = gtk_label_get_text(GTK_LABEL(msg_label));
    gtk_label_set_text(GTK_LABEL(msg_label), plain_msg);

    g_print("Clicked : %d\n", target_id);
    select_target_id = target_id;
    main_session->service->get_history_message(main_session->service, main_session->user, target_id);
    return FALSE;
}

GtkWidget* create_contact_item_with_click(int id,
                                          const char *title,
                                          const char *message,
                                          const char *time_str)
{
    GtkWidget *event_box = gtk_event_box_new();
    g_object_set_data(G_OBJECT(event_box), "id", GINT_TO_POINTER(id));

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    // Title
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);
    gtk_label_set_markup(GTK_LABEL(title_label), g_markup_printf_escaped("<b>%s</b>", title));

    // Message
    char short_msg[64];
    if (strlen(message) > 20) {
        snprintf(short_msg, sizeof(short_msg), "%.20s...", message);
    } else {
        snprintf(short_msg, sizeof(short_msg), "%s", message);
    }

    GtkWidget *msg_label = gtk_label_new(short_msg);
    gtk_label_set_xalign(GTK_LABEL(msg_label), 0.0);

    // Time
    GtkWidget *time_label = gtk_label_new(time_str);

    // Pack layout
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), msg_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), time_label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(event_box), hbox);
    g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_contact_item_click), NULL);
    gtk_widget_show_all(event_box);

    return event_box;
}

// Hàm xóa một contact dựa trên ID
void contact(int id) {
    //Clear chat are
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_view));
    gtk_text_buffer_set_text(buffer, "", -1);
    // Tìm widget tương ứng với ID trong hash table
    GtkWidget *widget = g_hash_table_lookup(contact_map, GINT_TO_POINTER(id));

    if (widget != NULL) {
        // Xóa widget khỏi contacts_box
        gtk_container_remove(GTK_CONTAINER(contacts_box), widget);

        // Xóa entry khỏi hash table
        g_hash_table_remove(contact_map, GINT_TO_POINTER(id));

        // Cập nhật giao diện
        gtk_widget_show_all(contacts_box);
    }
}

void update_or_create_contact(int id, const char *title, const char *message, long time, bool isGroup, bool bold) {
    int map_id = isGroup && id > 0 ? -id : id;
    time_t t = (time_t)time;
    struct tm *tm_info = gmtime(&t);
    char buffer[16];
    if (tm_info) {
        strftime(buffer, sizeof(buffer), "%H:%M", tm_info);
    } else {
        strncpy(buffer, "--:--", sizeof(buffer));
    }

    GtkWidget *existing_widget = g_hash_table_lookup(contact_map, GINT_TO_POINTER(map_id));

    if (existing_widget != NULL) {
        // Đã có rồi → chỉ cần update message + time + đưa lên đầu
        GtkWidget *vbox = get_vbox_from_event_box(existing_widget);
        GtkWidget *title_label = get_label_from_vbox(vbox, 0);
        GtkWidget *msg_label = get_label_from_vbox(vbox, 1);
        GtkWidget *time_label = get_time_label_from_event_box(existing_widget);

        // Rút gọn message
        char short_msg[64];
        if (message && strlen(message) > 20) {
            snprintf(short_msg, sizeof(short_msg), "%.20s...", message);
        } else {
            snprintf(short_msg, sizeof(short_msg), "%s", message ? message : "");
        }
        if (bold) {
            gtk_label_set_markup(GTK_LABEL(msg_label), g_markup_printf_escaped("<b>%s</b>", short_msg));
        } else {
            gtk_label_set_text(GTK_LABEL(msg_label), short_msg);
        }
        gtk_label_set_text(GTK_LABEL(time_label), buffer);


        log_message(INFO, "INFO update message %d %s", select_target_id, message);
        // Đưa lên đầu danh sách
        if (GTK_IS_WIDGET(existing_widget) && gtk_widget_get_parent(existing_widget) == contacts_box) {
            gtk_box_reorder_child(GTK_BOX(contacts_box), existing_widget, 0);
        } else {
            log_message(ERROR, "Widget is not valid or not in contacts_box");
        }

    } else {
        // Mới hoàn toàn — cần title để tạo widget
        if (!title) {
            log_message(ERROR, "Cannot create new contact: title is NULL");
            return;
        }

        GtkWidget *widget = create_contact_item_with_click(map_id, title, message, buffer);

        if (!widget) {
            log_message(ERROR, "create_contact_item_with_click failed");
            return;
        }
        gtk_box_pack_start(GTK_BOX(contacts_box), widget, FALSE, FALSE, 2);
        g_hash_table_insert(contact_map, GINT_TO_POINTER(map_id), widget);
    }

    gtk_widget_show_all(contacts_box);
}


// Hàm khởi tạo bảng băm contact_map (gọi một lần khi start)
void init_contact_map() {
    contact_map = g_hash_table_new(g_direct_hash, g_direct_equal);
}

void get_history()
{
    if (main_session && main_session->service)
    {
        main_session->service->get_history(main_session->service, main_session->user);
    }

}

//################---------search box-----------------------
// Global container và bảng băm nếu cần (ở đây chúng ta dùng search_results_box để chứa kết quả)
GtkWidget *search_results_box = NULL;
GtkWidget *search_box = NULL;

User* search_users_from_server(const gchar *keyword, int *out_count) {
    static User dummy_users[] = {
        {1, "Tran Van A"}, {2, "Tran Thi B"}, {3, "Nguyen Van C"}, {4, "Le Van D"}
    };

    static User results[10];
    int count = 0;

    gchar *lower_keyword = g_ascii_strdown(keyword, -1);

    for (int i = 0; i < 4; ++i) {
        gchar *lower_username = g_ascii_strdown(dummy_users[i].username, -1);
        if (g_strrstr(lower_username, lower_keyword)) {
            results[count++] = dummy_users[i];
        }
        g_free(lower_username);
    }

    g_free(lower_keyword);

    *out_count = count;
    return results;
}
/* ======== Hàm xử lý khi click vào một search item ======== */
static gboolean on_search_item_clicked(GtkWidget *event_box, GdkEventButton *event, gpointer user_data) {
    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(event_box), "id"));
    const char *username = (const char*)g_object_get_data(G_OBJECT(event_box), "username");
    // Clear nội dung ô nhập tìm kiếm
    GList *search_box_children = gtk_container_get_children(GTK_CONTAINER(search_box));
    GtkWidget *search_entry = g_list_nth_data(search_box_children, 1);
    if (search_entry && GTK_IS_ENTRY(search_entry)) {
        gtk_entry_set_text(GTK_ENTRY(search_entry), "");
    }
    g_list_free(search_box_children);

    // Xoá kết quả tìm kiếm
    GList *children = gtk_container_get_children(GTK_CONTAINER(search_results_box));
    for (GList *iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    // Kiểm tra xem user đã có trong danh sách contact chưa
    GtkWidget *contact_widget = g_hash_table_lookup(contact_map, GINT_TO_POINTER(id));

    if (contact_widget) {
        // Nếu đã tồn tại, scroll tới widget đó và kích hoạt nó
        GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(contacts_scroll));

        // Lấy vị trí của widget trong container
        GtkAllocation allocation;
        gtk_widget_get_allocation(contact_widget, &allocation);

        // Set giá trị adjustment để scroll tới widget
        gtk_adjustment_set_value(adj, allocation.y);
        log_message(INFO, "ITEM Click");
        on_contact_item_click(contact_widget, NULL, NULL);
    } else {
        // Sử dụng username từ search item thay vì tạo title mới
        // Tạo contact mới và thêm vào đầu danh sách
        GtkWidget *new_contact = create_contact_item_with_click(
            id, username ? username : "Unknown User", "Tin nhắn mới", "Bây giờ");

        // Thêm vào đầu danh sách
        gtk_box_pack_start(GTK_BOX(contacts_box), new_contact, FALSE, FALSE, 2);
        gtk_box_reorder_child(GTK_BOX(contacts_box), new_contact, 0);

        // Thêm vào bảng băm để dễ tìm kiếm sau này
        g_hash_table_insert(contact_map, GINT_TO_POINTER(id), new_contact);

        // Hiển thị widget mới
        gtk_widget_show_all(new_contact);

        // Kích hoạt contact mới
        log_message(INFO, "ITEM Click");
        on_contact_item_click(new_contact, NULL, NULL);
    }

    return FALSE;  // Trả về FALSE để cho phép các handler khác xử lý event
}

/* ======== Hàm tạo 1 search item ======== */
GtkWidget* create_search_item(int id, const char *title) {
    GtkWidget *button = gtk_button_new_with_label(title ? title : "Unknown User");

    // Gán data vào button
    g_object_set_data(G_OBJECT(button), "id", GINT_TO_POINTER(id));
    g_object_set_data_full(G_OBJECT(button), "username", g_strdup(title ? title : "Unknown User"), g_free);

    // Bỏ viền nếu muốn giống label hơn
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

    // Căn trái text trong button
    GtkWidget *label = gtk_bin_get_child(GTK_BIN(button));
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 10);

    // Kết nối sự kiện
    g_signal_connect(button, "button-press-event", G_CALLBACK(on_search_item_clicked), NULL);
    return button;
}

/* ======== Hàm cập nhật kết quả tìm kiếm ======== */
void update_search_user_results(User *users, int count) {
    // Xoá kết quả cũ
    GList *children = gtk_container_get_children(GTK_CONTAINER(search_results_box));
    for (GList *iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    // Tạo nút cho mỗi kết quả
    for (int i = 0; i < count; ++i) {
        if (users[i].id == main_session->user->id) continue;
        //GtkWidget *button = gtk_button_new_with_label(users[i].username);
        GtkWidget *button = create_search_item(users[i].id,users[i].username);
        // g_signal_connect_data(button, "clicked", G_CALLBACK(on_search_item_clicked),
        //                       GINT_TO_POINTER(users[i].id), NULL, 0);
        gtk_box_pack_start(GTK_BOX(search_results_box), button, FALSE, FALSE, 0);
    }

    gtk_widget_show_all(search_results_box);
}

void on_search_changed(GtkEntry *entry, gpointer user_data) {
    const gchar *text = gtk_entry_get_text(entry);
    main_session->service->search_users(main_session->service, text);
}

/* ======== Hàm k;hởi tạo giao diện của cột Search ======== */
GtkWidget* create_search_area(GtkWidget *grid) {
    search_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_grid_attach(GTK_GRID(grid), search_box, 0, 1, 1, 1);

    GtkWidget *search_label = gtk_label_new("Search");
    gtk_box_pack_start(GTK_BOX(search_box), search_label, FALSE, FALSE, 0);

    GtkWidget *search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(search_box), search_entry, FALSE, FALSE, 0);

    // Gắn sự kiện khi người dùng gõ
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), NULL);

    // Khu vực hiển thị kết quả
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    search_results_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(scroll), search_results_box);
    gtk_box_pack_start(GTK_BOX(search_box), scroll, TRUE, TRUE, 0);

    return search_box;
}

/* Hàm tạo và hiển thị giao diện chat */
void show_chat_window() {
    g_print("Creating main chat window...\n");
    // Tạo cửa sổ chính
    GtkWidget *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    set_current_ui(widget);
    gtk_window_set_title(GTK_WINDOW(widget), "Chat Application");
    gtk_window_set_default_size(GTK_WINDOW(widget), 1200, 600);
    gtk_window_set_position(GTK_WINDOW(widget), GTK_WIN_POS_CENTER);

    // Tạo lưới chính với 4 cột (theo yêu cầu)
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(widget), grid);
    // Column 0
    GtkWidget *search_area = create_search_area(grid);

    // Column 1
    contacts_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(contacts_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(contacts_scroll, 250, 400);
    gtk_grid_attach(GTK_GRID(grid), contacts_scroll, 1, 1, 1, 1);

    contacts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(contacts_scroll), contacts_box);


    //Column 2
    GtkWidget *chat_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chat_scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(chat_scrolled, TRUE);
    gtk_widget_set_hexpand(chat_scrolled, TRUE);
    gtk_grid_attach(GTK_GRID(grid), chat_scrolled, 2, 1, 1, 1);

    // Chat Area
    chat_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat_view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(chat_scrolled), chat_view);

    GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(grid), message_box, 2, 2, 1, 1);
    GtkWidget *message_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(message_box), message_entry, TRUE, TRUE, 0);
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(message_box), send_button, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(send_button), "message_entry", message_entry);
    g_object_set_data(G_OBJECT(send_button), "chat_view", chat_view);

    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), NULL);
    g_signal_connect(message_entry, "activate", G_CALLBACK(on_send_button_clicked), send_button);

    //Column 3
    GtkWidget *func_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_grid_attach(GTK_GRID(grid), func_box, 3, 1, 1, 2);

    GtkWidget *create_group_button = gtk_button_new_with_label("Create Group");
    g_signal_connect(create_group_button, "clicked", G_CALLBACK(on_create_group_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(func_box), create_group_button, FALSE, FALSE, 0);

    GtkWidget *join_group_button = gtk_button_new_with_label("Join Group");
    g_signal_connect(join_group_button, "clicked", G_CALLBACK(on_join_group_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(func_box), join_group_button, FALSE, FALSE, 0);

    GtkWidget *leave_group_button = gtk_button_new_with_label("Leave Group");
    g_signal_connect(leave_group_button, "clicked", G_CALLBACK(on_leave_group_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(func_box), leave_group_button, FALSE, FALSE, 0);
    GtkWidget *delete_group_button = gtk_button_new_with_label("Delete Group");
    g_signal_connect(delete_group_button, "clicked", G_CALLBACK(on_delete_group_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(func_box), delete_group_button, FALSE, FALSE, 0);

    GtkWidget *logout_button = gtk_button_new_with_label("Log Out");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(on_log_out_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(func_box), logout_button, FALSE, FALSE, 0);

    // Hiển thị tất cả widget
    gtk_widget_show_all(widget);

    g_signal_connect(widget, "destroy", G_CALLBACK(force_exit), NULL);
}

gboolean g_on_show_home_window(gpointer data)
{
    show_chat_window();
    init_contact_map();
    //Reset contact columns
    get_history();
    return FALSE;
}

gboolean g_on_update_search_user(const gpointer user_data) {
    UserListData *data = (UserListData *)user_data;
    int count = data->count;
    User *users = data->users;
    update_search_user_results(users, count);
    for (int i = 0; i < count; ++i) {
        free(users[i].username);
    }
    free(users);
    free(data);

    return FALSE;
}

gboolean g_on_update_history_contact(const gpointer user_data) {
    ChatMessage *data = (ChatMessage *)user_data;
    if (data)
    {
        update_or_create_contact(data->sender_id, data->target_name, data->content, data->timestamp, data->is_group_message, data->noti_message);
        if ((!data->is_group_message && data->sender_id == select_target_id) ||
            (data->is_group_message && data->sender_id == -select_target_id))
        {
            ChatMessage *msg = g_new0(ChatMessage, 1);
            msg->sender_id = data->sender_id;
            msg->sender_name = g_strdup(data->sender_name);
            msg->content = g_strdup(data->content);
            msg->timestamp = time(NULL);
            append_chat_message(msg);
            free(data);
        }
    }

    return FALSE; // trả FALSE để chỉ chạy 1 lần
}

gboolean g_on_load_history_message(const gpointer data)
{
    ChatMessageList *list = (ChatMessageList *)data;
    if (list && list->history)
    {
        load_chat_history(list->history, list->count);
        free(data);
    }
    return false;
}

gboolean g_on_history_contact(const gpointer data)
{
    int id = GPOINTER_TO_INT(data);
    if (id < 0)
    {
        contact(id);
    }
    return false;
}




