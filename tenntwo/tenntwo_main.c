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
#include <10n2_tf_pi.h>
#include <10n2_menu_handler.h>

int main(int argc, char *argv[])
{

  struct cam_req cam_r = {5, 1000};
  struct imu_req imu_r = {5, 100};
  struct gnss_req gnss_r = {5000, 1000};
  struct tf_pi_req tf_pi_r = {50, 1000};
  //  cam_init();
  imu_init();
  // gnss_init();
  aud_init();
  btn_init();
  menu_handler_init();
  send_aud_seq(startup_jingle, STARTUP_JINGLE_LEN);
  // tf_pi_init();

  // send_tf_pi_req(tf_pi_r);
  // send_gnss_req(gnss_r);
  // send_cam_req(cam_r);
  // send_imu_req(imu_r);

  while (1)
  {
    nxsig_usleep(15000); /* usecs (arbitrary) */
    if (current_menu == top && current_submenu == top_quit)
      break;
  }

  send_aud_seq(shutdown_jingle, SHUTDOWN_JINGLE_LEN);
  // cam_teardown();
  // gnss_teardown();
  //   imu_teardown();
  menu_handler_teardown();
  aud_teardown();
  btn_teardown();

  return 0;
}
