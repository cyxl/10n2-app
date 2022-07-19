
/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <arch/board/board.h>

#include <imu.h>
#include <nuttx/clock.h>
#include <10n2_menu_handler.h>
#include <10n2_imu.h>
#include <10n2_aud.h>
#include <10n2_btn.h>
#include <10n2_cam.h>
#include <10n2_gnss.h>
#include <10n2_tf_pi.h>
#include <10n2_rec.h>
#include <10n2_dp.h>

#define INF_CONF .5
#define YSLOPE_MAX 200
#define YSLOPE_MIN -200
#define XSLOPE_MAX 200
#define XSLOPE_MIN -200
#define ZSLOPE_MAX 200
#define ZSLOPE_MIN -200

static bool menu_handler_running = true;

static pthread_t menu_handler_th;

struct tf_req tf_r = {1, 0};
struct cam_req cam_nowrite_bw_r = {1, 900, 192, 12, 288, 108, 0, ""};
struct cam_req cam_hands_bw_r = {2, 0, 192, 12, 288, 108, 0, "hds"};
struct cam_req cam_cell_bw_r = {2, 0, 192, 12, 288, 108, 0, "cell"};
struct cam_req cam_none_bw_r = {2, 0, 192, 12, 288, 108, 0, "none"};
struct rec_req rec_open_r = {0, 0, rec_open, 0};
struct rec_req rec_close_r = {0, 0, rec_close, 0};
struct rec_req rec_write_r = {1, 0, rec_write, 0};

#define CAM_PERIOD 45
#define INF_PERIOD 45
#define POS_PERIOD 1

void update_service(uint8_t last_submenu, uint32_t tick)
{
    if (current_menu == train)
    {
        if (current_submenu == cam_hands_on)
        {
            if ((tick % CAM_PERIOD) == 0)
                send_cam_req(cam_hands_bw_r);
        }
        else if (current_submenu == cam_cell_on)
        {
            if ((tick % CAM_PERIOD) == 0)
                send_cam_req(cam_cell_bw_r);
        }
        else if (current_submenu == cam_none_on)
        {
            if ((tick % CAM_PERIOD) == 0)
                send_cam_req(cam_none_bw_r);
        }
    }
    else if (current_menu == inf)
    {
        if (current_submenu == inf_record && last_submenu != current_submenu)
        {
            // send open
            send_rec_req(rec_open_r);
        }
        if (current_submenu != inf_record && last_submenu == inf_record)
        {
            // send close
            send_rec_req(rec_close_r);
        }
        if (current_submenu == inf_off)
        {
            return;
        }
        if ((tick % INF_PERIOD) == 0)
        {
            send_cam_req(cam_nowrite_bw_r);
            send_tf_req(tf_r);

            if (current_submenu == inf_record)
            {
                // send record
                send_rec_req(rec_write_r);
            }

            // Process current inferences
            if (current_conf > INF_CONF)
            {
                switch (current_inf)
                {
                case TF_CELL:
                    printf("CELL %f\n", current_conf);
                    send_aud_seq(tf_cell_j, TF_CELL_J_LEN);
                    break;
                case TF_NOHANDS:
                    printf("NOHANDS %f\n", current_conf);
                    send_aud_seq(tf_none_j, TF_NONE_J_LEN);
                    break;
                default:
                    break;
                }
            }
        }
        if (current_y_slope >= YSLOPE_MAX)
        {
            send_aud_seq(acceleration_j_len, ACCELERATION_J_LEN);
        }
        else if (current_y_slope <= YSLOPE_MIN)
        {
            send_aud_seq(deceleration_j_len, DECELERATION_J_LEN);
        }
        if (current_z_slope >= ZSLOPE_MAX)
        {
            send_aud_seq(sharp_turn_j, SHARP_TURN_J_LEN);
        }
        else if (current_z_slope <= ZSLOPE_MIN)
        {
            send_aud_seq(sharp_turn_j, SHARP_TURN_J_LEN);
        }
        // TODO add x for pot holes
    }
}

void *_menu_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */
    uint8_t last_menu = 0;
    uint8_t last_submenu = 0;

    struct timespec del_sleep
    {
        0, 1 * (long)1e6
    };

    uint32_t tick = 0;
    while (menu_handler_running)
    {
        update_service(last_submenu, tick++);
        last_menu = current_menu;
        last_submenu = current_submenu;
        nanosleep(&del_sleep, NULL);
    }
    return NULL;
}

bool menu_handler_init(void)
{
    printf("menu handler init\n");
    menu_handler_running = true;
    pthread_create(&menu_handler_th, NULL, &_menu_run, NULL);

    return true;
}

bool menu_handler_teardown(void)
{
    printf("menu handler teardown\n");
    menu_handler_running = false;
    pthread_join(menu_handler_th, NULL);
    return true;
}
