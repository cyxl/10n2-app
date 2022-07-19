
#ifndef TENN2_DP_H
#define TENN2_DP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <imu_data.h>

    bool dp_init(void);
    bool dp_teardown(void);

    extern float current_x_slope;
    extern float current_y_slope;
    extern float current_z_slope;

#ifdef __cplusplus
}
#endif
#endif