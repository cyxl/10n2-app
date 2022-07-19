
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
// BWS#include <arm_math.h>

static bool dp_running = true;

//Globals
float current_x_slope;
float current_y_slope;
float current_z_slope;

static pthread_t dp_th_consumer;

float sum(float *vals, uint8_t num)
{
    float ret = 0.;
    for (int i = 0; i < num; i++)
        ret += vals[i];
    return ret;
}

void product(float *x, float *y, float *res, uint8_t num)
{
    for (int i = 0; i < num; i++)
        res[i] = x[i] * y[i];
}

void sqr(float *x, float *res, uint8_t num)
{
    for (int i = 0; i < num; i++)
        res[i] = x[i] * x[i];
}

static float x_vals[IMU_SAMPLE_SIZE]; 
float get_slope(float* vals,uint8_t size)
{
    // TODO lock
    float xy_prods[size];
    float x_sqrs[size];
    float xsum;
    float ysum;
    product(x_vals,vals,xy_prods,size);
    xsum = sum(x_vals,size);
    ysum = sum(vals,size);
    sqr(x_vals,x_sqrs,size);

    return ((size * sum(xy_prods,size)) - (xsum * ysum)) / 
           ((size * sum(x_sqrs,size))-(xsum*xsum));

}
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

        float *acx = get_latest_imu_samples(0);
        float *acy = get_latest_imu_samples(1);
        float *acz = get_latest_imu_samples(2);
        current_x_slope = get_slope(acx,IMU_SAMPLE_SIZE);
        current_y_slope = get_slope(acy,IMU_SAMPLE_SIZE);
        current_z_slope = get_slope(acz,IMU_SAMPLE_SIZE);
    }
    printf("dp done!\n");
    return NULL;
}

bool dp_init(void)
{
    printf("dp init\n");
    dp_running = true;
    pthread_create(&dp_th_consumer, NULL, &_dp_run, NULL);

    for (int i=1;i<=IMU_SAMPLE_SIZE;i++)
       x_vals[i] = (float)i; 
    return true;
}

bool dp_teardown(void)
{
    printf("dp teardown\n");
    dp_running = false;
    pthread_join(dp_th_consumer, NULL);
    return true;
}
