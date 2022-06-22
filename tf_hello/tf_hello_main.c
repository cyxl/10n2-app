#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <nuttx/signal.h>

#include <main_functions.h>
#include <imu_data.h>
#include <imu.h>
#include <t_test.h>
#include <10n2_gnss.h>
#include <10n2_aud.h>
#include <image_provider.h>
#include <camera_fileutil.h>

#include <arch/chip/gnss.h>
#define MY_GNSS_SIG 18

static void debug_log_printf(const char *s)
{
  printf("bws %s ", s);
}

char *DEVNAME = "/dev/imu0";
char g_data[sizeof(struct vel_gyro_s)];

int main(int argc, char *argv[])
{
  // beep
  if (!aud_init()){
    printf("audio init failure!\n");
  }
  if (!aud_beep(1,-40,300))
  {
    printf("audio beep failure!\n");
  }
  nxsig_usleep(5000000); /* usecs (arbitrary) */
  if (!aud_teardown())
  {
    printf("audio teardown failure!\n");
  }

  // Camera
 unsigned char* buf =(unsigned char*)memalign(32,96*96);
  printf("getting image \n");
  spresense_getimage(buf);
  printf("init futil \n");
  futil_initialize();
  printf("writing image\n");
  futil_writeimage(buf,96*96,"RGB");
  pthread_t test_t;

  t_test_start(&test_t);
  printf("zzzzz\n");
  nxsig_usleep(5000000); /* usecs (arbitrary) */
  t_test_stop(&test_t);
  printf("Start Tensorflow example \n");

  printf("done\n");

  return 0;

  /* Register callback for printing debug log */

  RegisterDebugLogCallback(debug_log_printf);

  // TF setup
  setup();

  int ret = 0;

  // IMU setup
  int fd;
  fd = open(DEVNAME, O_RDONLY);
  read(fd, &g_data, sizeof(struct vel_gyro_s));
  if (fd < 0)
  {
    printf("Device %s open failure. %d\n",
           DEVNAME, fd);
    return -1;
  }

  // GNSS setup
  int gnss_fd;
  int posperiod = 200;
  sigset_t mask;
  struct cxd56_gnss_signal_setting_s setting;
  printf("Hello, GNSS(USE_SIGNAL) SAMPLE!!\n");

  /* Get file descriptor to control GNSS. */

  gnss_fd = open("/dev/gps", O_RDONLY);
  if (gnss_fd < 0)
  {
    printf("open error:%d,%d\n", gnss_fd, errno);
    return -ENODEV;
  }

  sigemptyset(&mask);
  sigaddset(&mask, MY_GNSS_SIG);
  ret = sigprocmask(SIG_BLOCK, &mask, NULL);
  if (ret != OK)
  {
    printf("sigprocmask failed. %d\n", ret);
  }

  /* Set the signal to notify GNSS events. */

  setting.fd = gnss_fd;
  setting.enable = 1;
  setting.gnsssig = CXD56_GNSS_SIG_GNSS;
  setting.signo = MY_GNSS_SIG;
  setting.data = NULL;

  ret = ioctl(gnss_fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
  if (ret < 0)
  {
    printf("signal error\n");
  }

  /* Set GNSS parameters. */

  ret = gnss_setparams(gnss_fd);
  if (ret != OK)
  {
    printf("gnss_setparams failed. %d\n", ret);
  }
  ret = ioctl(gnss_fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
  {
    printf("start GNSS ERROR %d\n", errno);
  }
  else
  {
    printf("start GNSS OK\n");
  }

  do
  {
    /* Wait for positioning to be fixed. After fixed,
     * idle for the specified seconds. */

    ret = sigwaitinfo(&mask, NULL);
    if (ret != MY_GNSS_SIG)
    {
      printf("sigwaitinfo error %d\n", ret);
      break;
    }

    /* Read and print POS data. */

    ret = read_and_print(gnss_fd);
    if (ret < 0)
    {
      break;
    }

    posperiod--;
    printf("pos period %d\n",posperiod);
  } while (posperiod > 0);

  if (ret < 0)
  {
    printf("stop GNSS ERROR\n");
  }
  else
  {
    printf("stop GNSS OK\n");
  }


  while (1)
  {

    // TF loop
    loop();

    nxsig_usleep(500000); /* usecs (arbitrary) */
    ret = read(fd, &g_data, sizeof(struct vel_gyro_s));
    if (ret < 0)
    {
      int errcode = errno;
      fprintf(stderr, "ERROR: read() failed: %d\n", errcode);
      return -errcode;
    }
    struct vel_gyro_s s = get_mpu_data((int16_t *)g_data);
    dump_data(s);
    ret = sigwaitinfo(&mask, NULL);
    if (ret != MY_GNSS_SIG)
    {
      printf("sigwaitinfo error %d\n", ret);
      break;
    }
    ret = read_and_print(gnss_fd);
    if (ret > 0)
      printf("yay\n");
  }

  ret = ioctl(gnss_fd, CXD56_GNSS_IOCTL_STOP, 0);
  return 0;
}
