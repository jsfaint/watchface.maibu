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

/*窗口ID, 通过该窗口ID获取窗口句柄*/
static int8_t g_app_mwd_window_id = -1;

/*小时分钟图层ID，通过该图层ID获取图层句柄*/
static int8_t g_app_mwd_hm_layer_id = -1;

/*显示小时分钟文本图层*/
#define MWD_HM_ORIGIN_X     5
#define MWD_HM_ORIGIN_Y     110
#define MWD_HM_SIZE_H       16
#define MWD_HM_SIZE_W       50

void display_bitmap(P_Window pwindow)
{
    if (p_window == NULL) {
        return;
    }

    GRect frame = {{0,0}, {128, 128}}; //Bitmap position
    GBitmap bitmap;

    res_get_user_bitmap(RES_BITMAP_RUN, &bitmap);

    LayerBitmap layer_bitmap = {bitmap, frame, GAlignCenter};

    P_Layer layer = app_layer_create_bitmap(&layer_bitmap);
    if (layer != NULL) {
        app_window_add_layer(pwindow, layer);
    }
}

void display_timestr(P_Window pwindow)
{
    GRect frame_hm = {{MWD_HM_ORIGIN_X, MWD_HM_ORIGIN_Y}, {MWD_HM_SIZE_H, MWD_HM_SIZE_W}};

    struct date_time t;
    app_service_get_datetime(&t);

    char time_str[20] = "";
    sprintf(time_str, "%d:%02d", t.hour, t.min);
    LayerText lt_hm = {time_str, frame_hm, GAlignTopLeft, U_ASCII_ARIAL_14, 0};

    P_Layer layer = app_layer_create_text(&lt_hm);
    if(layer != NULL) {
        g_app_mwd_hm_layer_id = app_window_add_layer(p_window, layer);
    }
}

void app_mwd_watch_time_change(enum SysEventType type, void *context)
{
    /*如果系统事件是时间更改*/
    if (type == SysEventTypeTimeChange)
    {
        /*根据窗口ID获取窗口句柄*/
        P_Window p_window = app_window_stack_get_window_by_id(g_app_mwd_window_id);
        if (NULL == p_window)
            return;

        P_Layer p_hm_layer = app_window_get_layer_by_id(p_window, g_app_mwd_hm_layer_id);
        if (NULL == p_hm_layer)
            return;

        struct date_time datetime;
        app_service_get_datetime(&datetime);
        char str[20] = "";
        sprintf(str, "%02d:%02d", datetime.hour, datetime.min);
        app_layer_set_text_text(p_hm_layer, str);
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
    display_timestr(p_window);

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
        g_app_mwd_window_id = app_window_stack_push(p_window);
    }
}
