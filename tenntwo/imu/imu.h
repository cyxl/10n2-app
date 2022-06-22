#ifndef IMU_H
#define IMU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <imu_data.h>
void dump_data(struct vel_gyro_s vg_d);

struct vel_gyro_s get_mpu_data(int16_t* data);
#ifdef __cplusplus
}
#endif

#endif