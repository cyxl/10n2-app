
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
#include <10n2_pmu_handler.h>
#include <10n2_imu.h>
#include <10n2_aud.h>

static bool pmu_handler_running = true;

static pthread_t pmu_handler_th;

#define ACCEL_THRESHOLD -1000

#define X_IND_LEN 1
static struct aud_req x_ind[X_IND_LEN] = {
    {1,-40, 300, 10},
};

void *_pmu_run(void *args)
{

    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */
    uint16_t x_cnt = 0;
    uint16_t y_cnt = 0;
    uint16_t z_cnt = 0;

    struct timespec del_sleep
    {
        0, 1 * (long)1e6
    };

    while (pmu_handler_running)
    {

        if (current_pmu.ac_x < ACCEL_THRESHOLD)
        {
            x_cnt++;
        }
        else if (current_pmu.ac_x >= ACCEL_THRESHOLD)
        {
            x_cnt = 0;
        }

        if (x_cnt >= 100)
        {
            send_aud_seq(x_ind, X_IND_LEN);
            x_cnt = 0;
        }
        nanosleep(&del_sleep, NULL);
    }
    return NULL;
}

bool pmu_handler_init(void)
{
    printf("pmu handler init\n");
    pmu_handler_running = true;
    pthread_create(&pmu_handler_th, NULL, &_pmu_run, NULL);

    return true;
}

bool pmu_handler_teardown(void)
{
    printf("pmu handler teardown\n");
    pmu_handler_running = false;
    pthread_join(pmu_handler_th, NULL);
    return true;
}
