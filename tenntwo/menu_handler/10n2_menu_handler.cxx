
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
#include <nuttx/arch.h>

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

#define INF_CONF .4

static bool menu_handler_running = true;

static pthread_t menu_handler_th;

struct tf_req tf_r = {1, 0};
struct cam_req cam_nowrite_bw_r = {1, 0, 192, 12, 288, 108, 0, ""};
struct cam_req cam_hands_bw_r = {1, 0, 192, 12, 288, 108, 0, "hds"};
struct cam_req cam_cell_bw_r = {1, 0, 192, 12, 288, 108, 0, "cell"};
struct cam_req cam_none_bw_r = {1, 0, 192, 12, 288, 108, 0, "none"};
struct rec_req rec_open_verbose_r = {0, 0, rec_open, rec_verbose, 0};
struct rec_req rec_open_terse_r = {0, 0, rec_open, rec_terse, 0};
struct rec_req rec_close_r = {0, 0, rec_close, rec_verbose, 0}; // dont care type
struct rec_req rec_verbose_r = {1, 0, rec_write, rec_verbose, 0};
struct rec_req rec_terse_r = {1, 0, rec_write, rec_terse, 0};

#define CAM_PERIOD 50
#define INF_PERIOD 50

#define WARN_WAIT 100

uint32_t accel_warn_time = 0;
uint32_t decel_warn_time = 0;
uint32_t turn_warn_time = 0;
uint32_t pothole_warn_time = 0;

bool pos_open = false;

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
        if (current_submenu != inf_rec_terse && current_submenu != inf_rec_verbose && pos_open)
        {
            // send close
            printf("sending close\n");
            send_rec_req(rec_close_r);
            pos_open = false;
        }
        if (current_submenu == inf_rec_verbose && last_submenu != current_submenu)
        {
            // send open
            send_rec_req(rec_open_verbose_r);
            pos_open = true;
        }
        if (current_submenu == inf_rec_terse && last_submenu != current_submenu)
        {
            // send open
            send_rec_req(rec_open_terse_r);
            pos_open = true;
        }
        if (current_submenu == inf_off)
        {
            return;
        }
        if ((tick % INF_PERIOD) == 0)
        {
            send_cam_req(cam_nowrite_bw_r);
            send_tf_req(tf_r);

            if (current_submenu == inf_rec_verbose)
            {
                // send record
                send_rec_req(rec_verbose_r);
            }
            if (current_submenu == inf_rec_terse)
            {
                // send record
                send_rec_req(rec_terse_r);
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
                case TF_BAD:
                    printf("BAD %f\n", current_conf);
                    send_aud_seq(tf_bad_j, TF_BAD_J_LEN);
                    break;
                default:
                    break;
                }
            }
        }
        if ((current_imu_bit & ACCEL_BIT) && ((tick - accel_warn_time) > WARN_WAIT))
        {
            printf("warning accel %f\n", current_y_slope);
            send_aud_seq(acceleration_j, ACCELERATION_J_LEN);
            accel_warn_time = tick;
        }
        else if ((current_imu_bit & DECEL_BIT) && ((tick - decel_warn_time) > WARN_WAIT))
        {
            printf("warning decel\n");
            send_aud_seq(deceleration_j, DECELERATION_J_LEN);
            decel_warn_time = tick;
        }
        else if ((current_imu_bit & LEFT_BIT) && ((tick - turn_warn_time) > WARN_WAIT))
        {
            printf("l turn\n");
            send_aud_seq(sharp_turn_j, SHARP_TURN_J_LEN);
            turn_warn_time = tick;
        }
        else if ((current_imu_bit & RIGHT_BIT) && ((tick - turn_warn_time) > WARN_WAIT))
        {
            printf("r turn\n");
            send_aud_seq(sharp_turn_j, SHARP_TURN_J_LEN);
            turn_warn_time = tick;
        }
        else if ((current_imu_bit & POTHOLE_BIT) && ((tick - pothole_warn_time) > WARN_WAIT))
        {
            printf("r stdev\n %f", current_x_stdev);
            //TODOsend_aud_seq(pothole_j, POTHOLE_J_LEN);
            pothole_warn_time = tick;
        }
    }
}

void *_menu_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */
    int cpu = up_cpu_index();
    printf("MENU CPU %d\n", cpu);

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
    cpu_set_t cpuset = 1 << 3;
    int rc = pthread_setaffinity_np(menu_handler_th, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }
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
