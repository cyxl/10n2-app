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

#include <sys/boardctl.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int tenntwo_main(int argc, char *argv[])
#endif
{

  printf("TNT start\n");
  boardctl(BOARDIOC_INIT, 0);

  struct imu_req imu_r = {1e6, 50}; 
  struct gnss_req gnss_r = {1e6, 500};
  struct tf_req tf_r = {200, 0};

  int cpu = up_cpu_index();
  printf("MAIN CPU %d\n",cpu);
  printf("NUM CPUS%d\n",CONFIG_SMP_NCPUS);
  aud_init();
  imu_init();
  gnss_init();
  btn_init();
  dp_init();
  cam_init();
  tf_pi_init();
  rec_init();
  menu_handler_init();

  send_aud_seq(startup_jingle, STARTUP_JINGLE_LEN);

  send_imu_req(imu_r);
  send_gnss_req(gnss_r);

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
  rec_teardown();
  tf_pi_teardown();

  return 0;
}
