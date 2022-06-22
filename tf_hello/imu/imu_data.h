#ifndef IMU_DATA_H
#define IMU_DATA_H
struct vel_gyro_s
{
  int16_t ac_x;
  int16_t ac_y;
  int16_t ac_z;
  float tmp;
  int16_t gy_x;
  int16_t gy_y;
  int16_t gy_z;
};

#endif