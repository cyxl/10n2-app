
#ifndef TENN2_IMU_H
#define TENN2_IMU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <imu_data.h>

struct imu_req{
    uint32_t num;
    int16_t delay;
};

bool imu_init(void);
bool imu_teardown(void);

bool send_imu_req(struct imu_req req);
extern struct vel_gyro_s current_pmu;
#ifdef __cplusplus
}
#endif
#endif