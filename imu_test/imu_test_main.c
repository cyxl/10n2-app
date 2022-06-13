/****************************************************************************
 * gyro/gyro_main.c
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

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

#include <nuttx/sensors/mpu60x0.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

char *DEVNAME = "/dev/imu0";

uint16_t IMU_SIGNO = 13;

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

/****************************************************************************
 * Private Data
 ****************************************************************************/

char g_data[sizeof(struct vel_gyro_s)];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void dump_data(struct vel_gyro_s vg_d)
{
  printf("\n\t Vx: %i \t Vy: %i \t Vz: %i\n",vg_d.ac_x,vg_d.ac_y,vg_d.ac_z);
  printf("\n\t temp: %f",vg_d.tmp);
  printf("\n\t Gx: %i \t Gy: %i \t Gz: %i\n",vg_d.gy_x,vg_d.gy_y,vg_d.gy_z);
}

static struct vel_gyro_s get_mpu_data(int16_t* data)
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

static int sensing_main(int fd)
{
  printf("sensing main");
  int ret;

  for (;;)
  {

    nxsig_usleep(500000); /* usecs (arbitrary) */
    ret = read(fd, &g_data, sizeof(struct vel_gyro_s));
    if (ret < 0)
    {
      int errcode = errno;
      fprintf(stderr, "ERROR: read() failed: %d\n", errcode);
      return -errcode;
    }
    struct vel_gyro_s s = get_mpu_data((int16_t*) g_data);

    dump_data(s);
  }

  ASSERT(ret == 0);

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * sensor_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  int fd;

  fd = open(DEVNAME, O_RDONLY);
  read(fd, &g_data, sizeof(struct vel_gyro_s));

  printf("Sensing start...\n");

  printf("opening...\n");
  printf("done opening...\n");
  if (fd < 0)
  {
    printf("Device %s open failure. %d\n",
           DEVNAME, fd);
    return -1;
  }

  sensing_main(fd);

  close(fd);

  return 0;
}
