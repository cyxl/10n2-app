
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
#include <sys/stat.h>
#include <10n2_dp.h>
#include <10n2_imu.h>
//BWS#include <arm_math.h>

static bool dp_running = true;

static pthread_t dp_th_consumer;

void *_dp_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
    /* Initialize the queue attributes */

    // TODO make size configurable
    struct timespec poll_sleep = {0, 100000000};
    /*
    float32_t acx_r;
    float32_t acy_r;
    float32_t acz_r;
    float32_t gyx_r;
    float32_t gyy_r;
    float32_t gyz_r;
    */

    while (dp_running)
    {
        nanosleep(&poll_sleep, NULL);

        float* acx = get_latest_imu_samples(0);
        float* acy = get_latest_imu_samples(1);
        float* acz = get_latest_imu_samples(2);
        float* gyx = get_latest_imu_samples(3);
        float* gyy = get_latest_imu_samples(4);
        float* gyz = get_latest_imu_samples(5);
        if (acx != NULL) 
        {
            /*
            arm_mean_f32(acx, IMU_SAMPLE_SIZE, &acx_r);
            arm_mean_f32(acy, IMU_SAMPLE_SIZE, &acy_r);
            arm_mean_f32(acz, IMU_SAMPLE_SIZE, &acz_r);
            arm_mean_f32(gyx, IMU_SAMPLE_SIZE, &gyx_r);
            arm_mean_f32(gyy, IMU_SAMPLE_SIZE, &gyy_r);
            arm_mean_f32(gyz, IMU_SAMPLE_SIZE, &gyz_r);
            */

        }
        //printf("means %f,%f,%f,%f,%f,%f\n",acx_r, acy_r, acz_r, gyx_r, gyy_r, gyz_r);
    }
    printf("dp done!\n");
    return NULL;
}

bool dp_init(void)
{
    printf("dp init\n");
    dp_running = true;
    pthread_create(&dp_th_consumer, NULL, &_dp_run, NULL);

    return true;
}

bool dp_teardown(void)
{
    printf("dp teardown\n");
    dp_running = false;
    pthread_join(dp_th_consumer, NULL);
    return true;
}
