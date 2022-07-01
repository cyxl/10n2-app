
#ifndef TENN2_IMU_H
#define TENN2_IMU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <imu_data.h>

#define IMU_BUF_SIZE 35
#define IMU_SAMPLE_SIZE 10

struct imu_req{
    uint32_t num;
    int16_t delay;
};

bool imu_init(void);
bool imu_teardown(void);

bool send_imu_req(struct imu_req req);
extern struct vel_gyro_s current_pmu;
extern float imu_ac_buf[2][6][IMU_BUF_SIZE]; //ping pong buffer for imu samples
float* get_latest_imu_samples(uint8_t sample_type);


#ifdef __cplusplus
}
#endif
#endif