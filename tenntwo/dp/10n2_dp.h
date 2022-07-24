
#ifndef TENN2_DP_H
#define TENN2_DP_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ACCEL_BIT 1 << 1
#define DECEL_BIT 1 << 2
#define LEFT_BIT 1 << 3
#define RIGHT_BIT 1 << 4
#define POTHOLE_BIT 1 << 5

#define YSLOPE_MAX 200
#include <imu_data.h>

    bool dp_init(void);
    bool dp_teardown(void);

    extern float current_x_slope;
    extern float current_y_slope;
    extern float current_z_slope;
    extern float current_x_stdev;
    extern float current_y_stdev;
    extern float current_z_stdev;
    extern uint32_t current_imu_bit;

#ifdef __cplusplus
}
#endif
#endif