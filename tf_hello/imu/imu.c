/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <nuttx/kmalloc.h>
#include <nuttx/signal.h>

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include <nuttx/sensors/mpu60x0.h>

#ifdef CONFIG_CXD56_SCU
#include <arch/chip/scu.h>
#endif

#include "cxd56_i2c.h"
#include <imu.h>
#include <imu_data.h>

#include <nuttx/sensors/mpu60x0.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void dump_data(struct vel_gyro_s vg_d)
{
  printf("\n\t Vx: %i \t Vy: %i \t Vz: %i\n",vg_d.ac_x,vg_d.ac_y,vg_d.ac_z);
  printf("\n\t temp: %f",vg_d.tmp);
  printf("\n\t Gx: %i \t Gy: %i \t Gz: %i\n",vg_d.gy_x,vg_d.gy_y,vg_d.gy_z);
}

struct vel_gyro_s get_mpu_data(int16_t* data)
{
  struct vel_gyro_s ret;

  ret.ac_x = ((0x00FF & data[0]) << 8) | ((0xFF00 & data[0]) >> 8);
  ret.ac_y = ((0x00FF & data[1]) << 8) | ((0xFF00 & data[1]) >> 8);
  ret.ac_z = ((0x00FF & data[2]) << 8) | ((0xFF00 & data[2]) >> 8);
  ret.tmp = (int16_t)(((0x00FF & data[3]) << 8) | ((0xFF00 & data[3]) >> 8)) / 340.00 + 36.53;
  ret.gy_x = ((0x00FF & data[4]) << 8) | ((0xFF00 & data[4]) >> 8);
  ret.gy_y = ((0x00FF & data[5]) << 8) | ((0xFF00 & data[5]) >> 8);
  ret.gy_z = ((0x00FF & data[6]) << 8) | ((0xFF00 & data[6]) >> 8);

  return ret;

}
