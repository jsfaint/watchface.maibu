/*
 * =====================================================================================
 *
 *       Filename:  wf_cd_run.c
 *         Author:  Jia Sui , jsfaint@gmail.com
 *        Created:  2015/09/06
 *
 *    Description:
 *        A watch face for qiuyuke, with "成都跑客" logo
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "maibu_sdk.h"
#include "maibu_res.h"

/* Definition */
#define BG_ORIGIN_X 0
#define BG_ORIGIN_Y 0
#define BG_HEIGHT   128
#define BG_WIDTH   128

#define LEFT_LAYER_ORIGIN_X     5
#define LEFT_LAYER_ORIGIN_Y     110
#define LEFT_LAYER_HEIGHT       16
#define LEFT_LAYER_WIDTH       50

#define RIGHT_LAYER_ORIGIN_X     72
#define RIGHT_LAYER_ORIGIN_Y     110
#define RIGHT_LAYER_HEIGHT       16
#define RIGHT_LAYER_WIDTH       50

/* Enumeration */
enum state {
    STATE_DATE,
    STATE_HEIGHT,
    STATE_TEMPR
};

static int8_t g_app_window_id = -1;
static int8_t g_app_left_layer_id = -1;
static int8_t g_app_right_layer_id = -1;
static uint8_t g_app_state = STATE_DATE;

void display_bitmap(P_Window pwindow)
{
    if (pwindow == NULL)
        return;

    GRect frame = {{BG_ORIGIN_X, BG_ORIGIN_Y}, {BG_HEIGHT, BG_WIDTH}}; //Bitmap position
    GBitmap bitmap;

    res_get_user_bitmap(RES_BITMAP_RUN, &bitmap);

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
    /*SportData data;*/

    switch (g_app_state) {
        case STATE_DATE:
            app_service_get_datetime(&t);
            sprintf(str, "%d/%d", t.mon, t.mday);

            g_app_state = STATE_HEIGHT;
            break;
        case STATE_HEIGHT:
            sprintf(str, "%.1fm", maibu_bsp_pressure_get_height());

            g_app_state = STATE_TEMPR;
            break;
        case STATE_TEMPR:
            sprintf(str, "%.1fC", maibu_bsp_pressure_get_temperature());

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
    GRect frame_hm = {{LEFT_LAYER_ORIGIN_X, LEFT_LAYER_ORIGIN_Y}, {LEFT_LAYER_HEIGHT, LEFT_LAYER_WIDTH}}; //hour:minute
    GRect frame_md = {{RIGHT_LAYER_ORIGIN_X, RIGHT_LAYER_ORIGIN_Y}, {RIGHT_LAYER_HEIGHT, RIGHT_LAYER_WIDTH}}; //month/day
    char str[20] = "";

    get_left_layer_str(str);
    LayerText lt_hm = {str, frame_hm, GAlignLeft, U_ASCII_ARIAL_14, 0};
    P_Layer layer_hm = app_layer_create_text(&lt_hm);
    if(layer_hm != NULL) {
        g_app_left_layer_id = app_window_add_layer(pwindow, layer_hm);
    }

    get_right_layer_str(str);
    LayerText lt_md = {str, frame_md, GAlignRight, U_ASCII_ARIAL_14, 0};
    P_Layer layer_md = app_layer_create_text(&lt_md);
    if(layer_md != NULL) {
        g_app_right_layer_id = app_window_add_layer(pwindow, layer_md);
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

        struct date_time datetime;
        app_service_get_datetime(&datetime);
        char str[20] = "";
        //Left Layer
        sprintf(str, "%d:%02d", datetime.hour, datetime.min);
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
}
