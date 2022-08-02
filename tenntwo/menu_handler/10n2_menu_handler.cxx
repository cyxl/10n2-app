
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

#define INF_CONF .5

static bool menu_handler_running = true;

static pthread_t menu_handler_th;

struct tf_req tf_r = {1, 0};
struct cam_req cam_nowrite_bw_r = {1, 0, 192, 12, 288, 108, 0, ""};
struct cam_req cam_hands_bw_r = {1, 0, 192, 12, 288, 108, 0, "hds"};
struct cam_req cam_cell_bw_r = {1, 0, 192, 12, 288, 108, 0, "cell"};
struct cam_req cam_none_bw_r = {1, 0, 192, 12, 288, 108, 0, "none"};
struct rec_req rec_open_verbose_r = {rec_open, rec_verbose, 0};
struct rec_req rec_open_terse_r = {rec_open, rec_terse, 0};
struct rec_req rec_close_r = {rec_close, rec_verbose, 0}; // dont care type

#define CAM_TRAIN_PERIOD 10 
#define INF_PERIOD 40

#define WARN_WAIT 100

uint32_t accel_warn_time = 0;
uint32_t decel_warn_time = 0;
uint32_t turn_warn_time = 0;
uint32_t pothole_warn_time = 0;

bool pos_open = false;

void update_service(uint8_t menu,uint8_t last_submenu, uint32_t tick)
{
    if (menu == train)
    {
        if (current_submenu == cam_hands_on)
        {
            if ((tick % CAM_TRAIN_PERIOD) == 0)
                send_cam_req(cam_hands_bw_r);
        }
        else if (current_submenu == cam_cell_on)
        {
            if ((tick % CAM_TRAIN_PERIOD) == 0)
                send_cam_req(cam_cell_bw_r);
        }
        else if (current_submenu == cam_none_on)
        {
            if ((tick % CAM_TRAIN_PERIOD) == 0)
                send_cam_req(cam_none_bw_r);
        }
    }
    else if (menu == inf)
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

            // Process current inferences
            if (current_conf > INF_CONF)
            {
                switch (current_inf)
                {
                case CELL_IDX:
                    printf("CELL %f\n", current_conf);
                    send_aud_seq(tf_cell);
                    break;
                case NONE_IDX:
                    printf("NOHANDS %f\n", current_conf);
                    send_aud_seq(tf_none);
                    break;
                case BAD_IDX:
                    printf("BAD %f\n", current_conf);
                    send_aud_seq(tf_bad);
                    break;
                default:
                    break;
                }
            }
        }
        if ((current_imu_bit & ACCEL_BIT) && ((tick - accel_warn_time) > WARN_WAIT))
        {
            printf("warning accel %f\n", current_y_slope);
            send_aud_seq(imu_accel);
            accel_warn_time = tick;
        }
        else if ((current_imu_bit & DECEL_BIT) && ((tick - decel_warn_time) > WARN_WAIT))
        {
            printf("warning decel\n");
            send_aud_seq(imu_decel);
            decel_warn_time = tick;
        }
        else if ((current_imu_bit & LEFT_BIT) && ((tick - turn_warn_time) > WARN_WAIT))
        {
            printf("l turn\n");
            send_aud_seq(imu_turn);
            turn_warn_time = tick;
        }
        else if ((current_imu_bit & RIGHT_BIT) && ((tick - turn_warn_time) > WARN_WAIT))
        {
            printf("r turn\n");
            send_aud_seq(imu_turn);
            turn_warn_time = tick;
        }
        else if ((current_imu_bit & POTHOLE_BIT) && ((tick - pothole_warn_time) > WARN_WAIT))
        {
            printf("r stdev\n %f", current_x_stdev);
            send_aud_seq(imu_pothole);
            pothole_warn_time = tick;
        }
    }
}

void *_menu_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */
    //int cpu = up_cpu_index();
    //intf("MENU CPU %d\n", cpu);

    uint8_t last_menu = 0;
    uint8_t last_submenu = 0;
    uint32_t tick = 0;

    while (menu_handler_running)
    {
        btn_wait();
        uint8_t menu=current_menu;
        btn_release();
        update_service(current_menu,last_submenu, tick++);
        last_menu = menu;
        btn_wait();
        last_submenu = current_submenu;
        btn_release();
        usleep(1 * 1e3);
    }
    return NULL;
}

bool menu_handler_init(void)
{
    printf("menu handler init\n");
    menu_handler_running = true;
    pthread_create(&menu_handler_th, NULL, &_menu_run, NULL);
    cpu_set_t cpuset = 1 << 2;
    int rc ;
    rc= pthread_setaffinity_np(menu_handler_th, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }

    return true;
}

bool menu_handler_teardown(void)
{
    printf("menu handler teardown\n");
    menu_handler_running = false;
    pthread_join(menu_handler_th, NULL);
    return true;
}
