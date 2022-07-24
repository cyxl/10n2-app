
#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>

#include <10n2_aud.h>
#include <10n2_gnss.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define GNSS_POLL_FD_NUM 1
#define GNSS_POLL_TIMEOUT_FOREVER -1
#define POLL_FD_NUM 1
#define POLL_TIMEOUT_FOREVER -1

static struct cxd56_gnss_positiondata_s posdat;

static bool gnss_running = true;
#define GNSS_QUEUE_NAME "/gnss_queue" /* Queue name. */
#define GNSS_QUEUE_PERMS ((int)(0644))
#define GNSS_QUEUE_MAXMSG 16                       /* Maximum number of messages. */
#define GNSS_QUEUE_MSGSIZE sizeof(struct gnss_req) /* Length of message. */
#define GNSS_QUEUE_ATTR_INITIALIZER ((struct mq_attr){GNSS_QUEUE_MAXMSG, GNSS_QUEUE_MSGSIZE, 0, 0})

#define GNSS_QUEUE_POLL ((struct timespec){0, 100000000})

static struct mq_attr gnss_attr_mq = GNSS_QUEUE_ATTR_INITIALIZER;
static pthread_t gnss_th_consumer;

struct cxd56_gnss_positiondata_s current_gnss;

static struct pollfd g_fds[GNSS_POLL_FD_NUM] = {{0}};

void *_gnss_q_read(void *args)
{
  (void)args; /* Suppress -Wunused-parameter warning. */
              /* Initialize the queue attributes */

  int cpu = up_cpu_index();
  printf("GNSS CPU %d\n", cpu);
  struct timespec poll_sleep = {0, 100000000};

  /* Create the message queue. The queue reader is NONBLOCK. */
  mqd_t r_mq = mq_open(GNSS_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, GNSS_QUEUE_PERMS, &gnss_attr_mq);
  if (r_mq < 0)
  {
    fprintf(stderr, "[CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
    return NULL;
  }

  printf("[CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

  unsigned int prio;
  ssize_t bytes_read;
  char buffer[GNSS_QUEUE_MSGSIZE];

  int gnss_fd;
  gnss_fd = open("/dev/gps", O_RDONLY);

  if (gnss_fd < 0)
  {
    printf("open error:%d,%d\n", gnss_fd, errno);
    return -ENODEV;
  }

  int ret = ioctl(gnss_fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
  {
    printf("start GNSS ERROR %d\n", errno);
    return false;
  }
  else
  {
    printf("start GNSS OK\n");
  }

  g_fds[0].fd = gnss_fd;
  g_fds[0].events = POLLIN;

  while (gnss_running)
  {

    bytes_read = mq_receive(r_mq, buffer, GNSS_QUEUE_MSGSIZE, &prio);
    struct gnss_req *r = (struct gnss_req *)buffer;

    if (bytes_read >= 0)
    {
      for (int i = 0; i < r->num; i++)
      {

        ret = poll(g_fds, POLL_FD_NUM, POLL_TIMEOUT_FOREVER);
        if (ret <= 0)
        {
          printf("poll error %d,%x,%x\n", ret,
                 g_fds[0].events, g_fds[0].revents);
          break;
        }
        if (g_fds[0].revents & POLLIN)
        {
          /* Read pos data. */

          ret = read(gnss_fd, &current_gnss, sizeof(current_gnss));
          if (ret < 0)
          {
            printf("Error read position data\n");
            break;
          }
          else if (ret != sizeof(current_gnss))
          {
            ret = ERROR;
            printf("Size error read position data\n");
            break;
          }
          else
          {
            ret = OK;
          }

          printf("UTC time : Hour:%d, minute:%d, sec:%d\n",
                 current_gnss.receiver.time.hour, current_gnss.receiver.time.minute,
                 current_gnss.receiver.time.sec);
          if (current_gnss.receiver.pos_dataexist)
          {
            /* Detect position. */
            printf("## POSITION: LAT %ld, LNG %ld \n", current_gnss.receiver.latitude,current_gnss.receiver.longitude);
            send_aud_seq(gnss_jingle,GNSS_JINGLE_LEN);
            break;
          }
          else
          {
            printf("## No Positioning Data\n");
          }
        }

        struct timespec gnss_sleep = {r->delay / 1000, (r->delay % 1000) * 1e6};
        nanosleep(&gnss_sleep, NULL);
      }
    }
    else
    {
      poll_sleep = GNSS_QUEUE_POLL;
      nanosleep(&poll_sleep, NULL);
    }
  }
  printf("gnss cleaning mq\n");
  ret = ioctl(gnss_fd, CXD56_GNSS_IOCTL_STOP, 0);
  mq_close(r_mq);
  // mq_unlink(GNSS_QUEUE_NAME);
  return NULL;
}

bool send_gnss_req(struct gnss_req req)
{
  mqd_t mq = mq_open(GNSS_QUEUE_NAME, O_WRONLY);
  if (mq < 0)
  {
    fprintf(stderr, "[sender]: Error, cannot open the queue: %s.\n", strerror(errno));
    return false;
  }
  if (mq_send(mq, (char *)&req, GNSS_QUEUE_MSGSIZE, 1) < 0)
  {
    fprintf(stderr, "[sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
  }

  mq_close(mq);
  return true;
}

bool gnss_init(void)
{
  printf("gnss init\n");
  current_gnss.receiver.type = NO_TYPE;
  gnss_running = true;

  cpu_set_t cpuset = 1 << 5;
  int rc = pthread_setaffinity_np(gnss_th_consumer, sizeof(cpu_set_t), &cpuset);
  if (rc != 0)
  {
    printf("Unable set CPU affinity : %d", rc);
  }
  pthread_create(&gnss_th_consumer, NULL, &_gnss_q_read, NULL);

  return true;
}

bool gnss_teardown(void)
{
  printf("gnss teardown\n");
  gnss_running = false;
  pthread_join(gnss_th_consumer, NULL);
  return true;
}

void double_to_dmf(double x, struct cxd56_gnss_dms_s *dmf)
{
  int b;
  int d;
  int m;
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

  dmf->sign = b;
  dmf->degree = d;
  dmf->minute = m;
  dmf->frac = f;
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

int read_gnss(int fd, struct gnss_data *d)
{
  int ret;
  ret = read(fd, &posdat, sizeof(posdat));
  if (ret < 0)
  {
    printf("read error\n");
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

  if (posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
  {
    d->date = posdat.receiver.date;
    d->time = posdat.receiver.time;
    d->latitude = posdat.receiver.latitude;
    d->longitude = posdat.receiver.longitude;
    d->type = posdat.sv->type;
    d->svid = posdat.sv->svid;
    d->stat = posdat.sv->stat;
    d->siglevel = posdat.sv->siglevel;
    return OK;
  }
  else
  {
    return ERROR;
  }

_err:
  return ret;
}

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
  // if (posdat.sv->invalid_cause == 0)
  {
    /* 2D fix or 3D fix.
     * Convert latitude and longitude into dmf format and print it. */

    double_to_dmf(posdat.receiver.latitude, &dmf);
    printf(">LAT %d.%d.%04ld\n", dmf.degree, dmf.minute, dmf.frac);

    double_to_dmf(posdat.receiver.longitude, &dmf);
    printf(">LNG %d.%d.%04ld\n", dmf.degree, dmf.minute, dmf.frac);

    printf(">    type %i    \n", posdat.sv->type);
    printf(">    svid %i    \n", posdat.sv->svid);
    printf(">    stat %i    \n", posdat.sv->stat);
    printf(">    siglevel %f    \n", posdat.sv->siglevel);
    ret = 1; // posfix
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
  int ret = 0;
  uint32_t set_satellite;
  struct cxd56_gnss_ope_mode_param_s set_opemode;

  /* Set the GNSS operation interval. */

  set_opemode.mode = 1;    /* Operation mode:Normal(default). */
  set_opemode.cycle = 100; /* Position notify cycle(msec step). */

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
