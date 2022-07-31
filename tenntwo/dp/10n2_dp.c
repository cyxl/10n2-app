
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
#include <arm_math.h>

static bool dp_running = true;

// Globals
float current_x_slope;
float current_y_slope;
float current_z_slope;
float current_x_stdev;
float current_y_stdev;
float current_z_stdev;

#define YSLOPE_MAX 400
#define YSLOPE_MIN -400
#define ZSLOPE_MAX 250
#define ZSLOPE_MIN -250
#define XSTDEV_MAX 1800

#define CLEAR_BIT 10

uint32_t current_imu_bit = 0;

static pthread_t dp_th_consumer;
uint64_t cnt = 0;

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
float get_slope(float *vals, uint8_t size)
{
    // TODO lock
    float xy_prods[size];
    float x_sqrs[size];
    float xsum;
    float ysum;
    product(x_vals, vals, xy_prods, size);
    xsum = sum(x_vals, size);
    ysum = sum(vals, size);
    sqr(x_vals, x_sqrs, size);

    return ((size * sum(xy_prods, size)) - (xsum * ysum)) /
           ((size * sum(x_sqrs, size)) - (xsum * xsum));
}
void *_dp_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */

    //int cpu = up_cpu_index();
    //printf("DP CPU %d\n", cpu);
    struct timespec poll_sleep = {0, 100000000};

    while (dp_running)
    {
        if ((cnt++ % CLEAR_BIT) == 0)
        {
            current_imu_bit = 0;
        }

        usleep(10 * 1e3);

        float *acx = get_latest_imu_samples(0);
        float *acy = get_latest_imu_samples(1);
        float *acz = get_latest_imu_samples(2);
        current_x_slope = get_slope(acx, IMU_SAMPLE_SIZE);
        current_y_slope = get_slope(acy, IMU_SAMPLE_SIZE);
        current_z_slope = get_slope(acz, IMU_SAMPLE_SIZE);

        // std deviations
        arm_std_f32(acx, IMU_SAMPLE_SIZE, &current_x_stdev);
        arm_std_f32(acy, IMU_SAMPLE_SIZE, &current_y_stdev);
        arm_std_f32(acz, IMU_SAMPLE_SIZE, &current_z_stdev);

        if (current_y_slope >= YSLOPE_MAX)
        {
            printf("==================accel\n");
            current_imu_bit |= ACCEL_BIT;
        }
        else if (current_y_slope <= YSLOPE_MIN)
        {
            printf("==================decel\n");
            current_imu_bit |= DECEL_BIT;
        }
        else if (current_z_slope >= ZSLOPE_MAX)
        {
            printf("==================left\n");
            current_imu_bit |= LEFT_BIT;
        }
        else if (current_z_slope <= ZSLOPE_MIN)
        {
            printf("==================right\n");
            current_imu_bit |= RIGHT_BIT;
        }
        else if (current_x_stdev >= XSTDEV_MAX)
        {
            printf("==================pothole\n");
            current_imu_bit |= POTHOLE_BIT;
        }
    }
    printf("dp done!\n");
    return NULL;
}

bool dp_init(void)
{
    printf("dp init\n");
    dp_running = true;
    pthread_create(&dp_th_consumer, NULL, &_dp_run, NULL);
    cpu_set_t cpuset = 1 << 2;
    int rc=0;
    rc= pthread_setaffinity_np(dp_th_consumer, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }

    for (int i = 1; i <= IMU_SAMPLE_SIZE; i++)
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
