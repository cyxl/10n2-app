
#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <10n2_gnss.h>
#include <arch/chip/gnss.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define GNSS_POLL_FD_NUM          1
#define GNSS_POLL_TIMEOUT_FOREVER -1
#define MY_GNSS_SIG               18

static struct cxd56_gnss_positiondata_s posdat;


void double_to_dmf(double x, struct cxd56_gnss_dms_s * dmf)
{
  int    b;
  int    d;
  int    m;
  double f;
  double t;

  if (x < 0)
    {
      b = 1;
      x = -x;
    }
  else
    {
      b = 0;
    }
  d = (int)x; /* = floor(x), x is always positive */
  t = (x - d) * 60;
  m = (int)t; /* = floor(t), t is always positive */
  f = (t - m) * 10000;

  dmf->sign   = b;
  dmf->degree = d;
  dmf->minute = m;
  dmf->frac   = f;
}

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int read_and_print(int fd)
{
  int ret;
  struct cxd56_gnss_dms_s dmf;

  /* Read POS data. */

  ret = read(fd, &posdat, sizeof(posdat));
  if (ret < 0)
    {
      printf("read error\n");
      goto _err;
    }
  else if (ret != sizeof(posdat))
    {
      ret = ERROR;
      printf("read size error\n");
      goto _err;
    }
  else
    {
      ret = OK;
    }

  /* Print POS data. */

  /* Print time. */

  printf(">Hour:%d, minute:%d, sec:%d, usec:%ld\n",
         posdat.receiver.time.hour, posdat.receiver.time.minute,
         posdat.receiver.time.sec, posdat.receiver.time.usec);
  if (posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
  //if (posdat.sv->invalid_cause == 0)
    {
      /* 2D fix or 3D fix.
       * Convert latitude and longitude into dmf format and print it. */


      double_to_dmf(posdat.receiver.latitude, &dmf);
      printf(">LAT %d.%d.%04ld\n", dmf.degree, dmf.minute, dmf.frac);

      double_to_dmf(posdat.receiver.longitude, &dmf);
      printf(">LNG %d.%d.%04ld\n", dmf.degree, dmf.minute, dmf.frac);

      printf(">    type %i    \n",posdat.sv->type);
      printf(">    svid %i    \n",posdat.sv->svid);
      printf(">    stat %i    \n",posdat.sv->stat);
      printf(">    siglevel %f    \n",posdat.sv->siglevel);
      ret = 1; //posfix
    }
  else
    {
      /* No measurement. */

      printf(">No Positioning Data\n");
   //   printf(">    %i    \n",posdat.sv->invalid_cause);
   //   printf(">    type %i    \n",posdat.sv->type);
   //   printf(">    svid %i    \n",posdat.sv->svid);
   //   printf(">    stat %i    \n",posdat.sv->stat);
    //  printf(">    siglevel %f    \n",posdat.sv->siglevel);
    }

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_setparams()
 *
 * Description:
 *   Set gnss parameters use ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_setparams(int fd)
{
  int      ret = 0;
  uint32_t set_satellite;
  struct cxd56_gnss_ope_mode_param_s set_opemode;

  /* Set the GNSS operation interval. */

  set_opemode.mode     = 1;     /* Operation mode:Normal(default). */
  set_opemode.cycle    = 100;  /* Position notify cycle(msec step). */

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SET_OPE_MODE, (uint32_t)&set_opemode);
  if (ret < 0)
    {
      printf("ioctl(CXD56_GNSS_IOCTL_SET_OPE_MODE) NG!!\n");
      goto _err;
    }

  /* Set the type of satellite system used by GNSS. */

  set_satellite = CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, set_satellite);
  if (ret < 0)
    {
      printf("ioctl(CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM) NG!!\n");
      goto _err;
    }

_err:
  return ret;
}
