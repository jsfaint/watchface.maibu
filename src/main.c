/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *         Author:  Jia Sui , jsfaint@gmail.com
 *        Created:  2015/09/07
 *
 * =====================================================================================
 */
#include "maibu_sdk.h"
#include "../build/maibu_res.h"

/* Definition */
#define BG_ORIGIN_X 0
#define BG_ORIGIN_Y 0
#define BG_HEIGHT   128
#define BG_WIDTH   128

#define LEFT_LAYER_ORIGIN_X     10
#define LEFT_LAYER_ORIGIN_Y     110
#define LEFT_LAYER_HEIGHT       16
#define LEFT_LAYER_WIDTH        59

#define RIGHT_LAYER_ORIGIN_X     63
#define RIGHT_LAYER_ORIGIN_Y     110
#define RIGHT_LAYER_HEIGHT       16
#define RIGHT_LAYER_WIDTH        58

/* Enumeration */
enum state {
    STATE_DATE,
    STATE_HEIGHT,
    STATE_TEMPR,
    STATE_SPORT
};

static int32_t g_app_window_id = -1;
static int8_t g_app_left_layer_id = -1;
static int8_t g_app_right_layer_id = -1;
static uint8_t g_app_state = STATE_DATE;

void display_bitmap(P_Window pwindow)
{
    if (pwindow == NULL)
        return;

    GRect frame = {{BG_ORIGIN_X, BG_ORIGIN_Y}, {BG_HEIGHT, BG_WIDTH}}; //Bitmap position
    GBitmap bitmap;

    res_get_user_bitmap(RES_BITMAP_BG, &bitmap);

    LayerBitmap layer_bitmap = {bitmap, frame, GAlignCenter};

    P_Layer layer = app_layer_create_bitmap(&layer_bitmap);
    if (layer != NULL) {
        app_window_add_layer(pwindow, layer);
    }
}

void get_left_layer_str(char *str)
{
    struct date_time t;
    app_service_get_datetime(&t);

    sprintf(str, "%d:%02d", t.hour, t.min);
}

void get_right_layer_str(char *str)
{
    struct date_time t;
    SportData data;
    float tmp1 = 0;
    float tmp2 = 0;

    switch (g_app_state) {
        case STATE_DATE:
            app_service_get_datetime(&t);
            sprintf(str, "%d/%d", t.mon, t.mday);

            g_app_state = STATE_HEIGHT;
            break;
        case STATE_HEIGHT:
            maibu_get_altitude(&tmp1, &tmp2);
            sprintf(str, "%.0f.%.0f米", tmp1, tmp2);

            g_app_state = STATE_TEMPR;
            break;
        case STATE_TEMPR:
            maibu_get_temperature(&tmp1);
            sprintf(str, "%.1f度", tmp1);

            g_app_state = STATE_SPORT;
            break;
        case STATE_SPORT:
            maibu_get_sport_data(&data, 0);
            sprintf(str, "%d步", data.step);

            g_app_state = STATE_DATE;
            break;
        default:
            sprintf(str, "Unknown");

            g_app_state = STATE_DATE;
            break;
    }
}

void init_text_layer(P_Window pwindow)
{
    GRect frame_left = {{LEFT_LAYER_ORIGIN_X, LEFT_LAYER_ORIGIN_Y}, {LEFT_LAYER_HEIGHT, LEFT_LAYER_WIDTH}}; //hour:minute
    GRect frame_right = {{RIGHT_LAYER_ORIGIN_X, RIGHT_LAYER_ORIGIN_Y}, {RIGHT_LAYER_HEIGHT, RIGHT_LAYER_WIDTH}}; //month/day
    char str[30] = "";

    get_left_layer_str(str);
    LayerText lt_left = {str, frame_left, GAlignLeft, U_ASCII_ARIAL_14, 0};
    P_Layer layer_left = app_layer_create_text(&lt_left);

    if(layer_left != NULL) {
        g_app_left_layer_id = app_window_add_layer(pwindow, layer_left);
    }

    get_right_layer_str(str);
    LayerText lt_right = {str, frame_right, GAlignRight, U_ASCII_ARIAL_14, 0};
    P_Layer layer_right = app_layer_create_text(&lt_right);

    if(layer_right != NULL) {
        g_app_right_layer_id = app_window_add_layer(pwindow, layer_right);
    }
}

void app_mwd_watch_time_change(enum SysEventType type, void *context)
{
    /*如果系统事件是时间更改*/
    if (type == SysEventTypeTimeChange)
    {
        /*根据窗口ID获取窗口句柄*/
        P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_app_window_id);
        if (NULL == p_window)
            return;

        P_Layer p_left_layer = app_window_get_layer_by_id(p_window, g_app_left_layer_id);
        if (NULL == p_left_layer)
            return;

        P_Layer p_right_layer = app_window_get_layer_by_id(p_window, g_app_right_layer_id);
        if (NULL == p_right_layer)
            return;

        char str[30] = "";
        //Left Layer
        get_left_layer_str(str);
        app_layer_set_text_text(p_left_layer, str);

        //Right Layer
        get_right_layer_str(str);
        app_layer_set_text_text(p_right_layer, str);

        app_window_update(p_window);
    }
}

P_Window init_mwd_window()
{
    P_Window p_window = app_window_create();
    if (NULL == p_window) {
        return NULL;
    }

    display_bitmap(p_window);
    init_text_layer(p_window);

    /*注册一个事件通知回调，当有时间改变时，立即更新时间*/
    maibu_service_sys_event_subscribe(app_mwd_watch_time_change);

    return p_window;
}

int main()
{
    /*创建显示时间窗口*/
    P_Window p_window = init_mwd_window();
    if (p_window != NULL)
    {
        /*放入窗口栈显示*/
        g_app_window_id = app_window_stack_push(p_window);
    }

    return 0;
}
