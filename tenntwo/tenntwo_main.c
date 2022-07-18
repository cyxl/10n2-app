#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <nuttx/signal.h>

#include <10n2_cam.h>
#include <10n2_aud.h>
#include <10n2_imu.h>
#include <10n2_btn.h>
#include <10n2_gnss.h>
#include <10n2_dp.h>
#include <10n2_tf_pi.h>
#include <10n2_menu_handler.h>

#include <sys/boardctl.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int tenntwo_main(int argc, char *argv[])
#endif
{

  printf("TNT start\n");
  boardctl(BOARDIOC_INIT, 0);

  struct cam_req cam_nowrite_bw_r = {2, 0, 160, 0, 320, 120, 0, ""};
  struct imu_req imu_r = {1e6, 100}; // TODO 5000 total secs.
  struct gnss_req gnss_r = {10, 100};
  struct tf_req tf_r = {500, 1000};
  cam_init();
  imu_init();
  gnss_init();
  aud_init();
  btn_init();
  dp_init();
  tf_pi_init();
  menu_handler_init();

  send_aud_seq(startup_jingle, STARTUP_JINGLE_LEN);

  send_imu_req(imu_r);
  while (1)
  {
    //   send_gnss_req(gnss_r);
    nxsig_usleep(1 * 1e6); /* usecs (arbitrary) */
    if (current_menu == top && current_submenu == top_quit)
      break;
  }

  send_aud_seq(shutdown_jingle, SHUTDOWN_JINGLE_LEN);
  dp_teardown();
  cam_teardown();
  gnss_teardown();
  imu_teardown();
  menu_handler_teardown();
  aud_teardown();
  btn_teardown();
  tf_pi_teardown();

  return 0;
}
