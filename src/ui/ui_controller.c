//
// Created by vawnwuyest on 09/04/2025.
//
#include "../include/ui_controller.h"

#include <log.h>

void on_show_ui(MAIN_UI_LEVEL level)
{
    switch (level)
    {
        case LOGIN:
            {
                show_login_window();
                break;
            }
        case LOGOUT:
            {
                show_logout_window();
                break;
            }
        case HOME:
            {
                show_home_window();
                break;
            }
        default:
            {
                show_notification_window(NOTI_LEVEL::ERROR, "No has windows %s", level);
                break;
            }
    }
}

void init(Session *session)
{
    if (session == NULL)
    {
        log_message(LogLevel::ERROR, "Null Sessions");
        return;
    }
    main_session = session;
}

void set_current_ui(GtkWidget *widget)
{
    if (widget == NULL) return;
    //Hide previous


    //Set new
    current_ui = widget;
}

void show_home_window()
{
    g_idle_add((GSourceFunc)on_show_home_window, NULL);
}
void show_login_window()
{
    g_idle_add((GSourceFunc)on_show_login_window, NULL);

}
void show_create_group_window()
{
    g_idle_add((GSourceFunc)on_show_create_window, NULL);

}
void show_join_group_window()
{
    g_idle_add((GSourceFunc)on_show_join_window, NULL);
}
void show_logout_window()
{
    g_idle_add((GSourceFunc)on_show_logout_window, NULL);
}
void show_notification_window(NOTI_LEVEL level, const char *content, ...)
{

}
void on_receive_message()
{

}
void on_load_history_message()
{

}