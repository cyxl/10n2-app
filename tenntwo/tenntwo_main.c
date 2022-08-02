#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <nuttx/signal.h>
#include <nuttx/arch.h>

#include <10n2_cam.h>
#include <10n2_aud.h>
#include <10n2_imu.h>
#include <10n2_btn.h>
#include <10n2_gnss.h>
#include <10n2_dp.h>
#include <10n2_tf_pi.h>
#include <10n2_menu_handler.h>
#include <10n2_rec.h>
#include <nuttx/arch.h>

#include <sys/boardctl.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int tenntwo_main(int argc, char *argv[])
#endif
{
 boardctl(BOARDIOC_INIT, 0);

  printf("TNT start\n");

  struct imu_req imu_r = {1e6, 150};
  struct gnss_req gnss_r = {1e6, 500};

  aud_init();
  imu_init();
  gnss_init();
  btn_init();
  dp_init();
  cam_init();
  tf_pi_init();
  rec_init();
  menu_handler_init();

  send_aud_seq(startup_j);

  send_imu_req(imu_r);
  send_gnss_req(gnss_r);

  while (1)
  {
    //send_gnss_req(gnss_r);
    nxsig_usleep(100 * 1e6); /* usecs (arbitrary) */
    printf("zzzz....\n");
    if (current_menu == top && current_submenu == top_quit)
      break;
  }

  send_aud_seq(shutdown_j);
  dp_teardown();
  cam_teardown();
  gnss_teardown();
  imu_teardown();
  menu_handler_teardown();
  aud_teardown();
  btn_teardown();
  rec_teardown();
  tf_pi_teardown();

  return 0;
}
