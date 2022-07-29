
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

static bool gnss_running = true;
#define GNSS_QUEUE_NAME "/gnss_queue" /* Queue name. */
#define GNSS_QUEUE_PERMS ((int)(0644))
#define GNSS_QUEUE_MAXMSG 16                       /* Maximum number of messages. */
#define GNSS_QUEUE_MSGSIZE sizeof(struct gnss_req) /* Length of message. */
#define GNSS_QUEUE_ATTR_INITIALIZER ((struct mq_attr){GNSS_QUEUE_MAXMSG, GNSS_QUEUE_MSGSIZE, 0, 0})

#define GNSS_QUEUE_POLL ((struct timespec){0, 100000000})

static struct mq_attr gnss_attr_mq = GNSS_QUEUE_ATTR_INITIALIZER;
static pthread_t gnss_th_consumer;

struct cxd56_gnss_positiondata_s received_gnss;
struct gnss_data current_gnss;

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
    fprintf(stderr, "[GNSS CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
    return NULL;
  }

  printf("[GNSS CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

  unsigned int prio;
  ssize_t bytes_read;
  char buffer[GNSS_QUEUE_MSGSIZE];

  int gnss_fd;
  gnss_fd = open("/dev/gps", O_RDONLY);

  if (gnss_fd < 0)
  {
    printf("open error:%d,%d\n", gnss_fd, errno);
    return NULL;
  }

  int ret = gnss_setparams(gnss_fd);
  if (ret < 0)
  {
    printf("set GNSS parms ERROR %d\n", errno);
    return NULL;
  }
  else
  {
    printf("set GNSS parms OK\n");
  }

  ret = ioctl(gnss_fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
  {
    printf("start GNSS ERROR %d\n", errno);
    return NULL;
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

          ret = read(gnss_fd, &received_gnss, sizeof(received_gnss));
          if (ret < 0)
          {
            printf("Error read position data\n");
            break;
          }
          else if (ret != sizeof(received_gnss))
          {
            ret = ERROR;
            printf("Size error read position data\n");
            break;
          }
          else
          {
            to_gnss(&received_gnss,&current_gnss);
            ret = OK;
          }

          printf("UTC time : Hour:%d, minute:%d, sec:%d\n",
                 received_gnss.receiver.time.hour, received_gnss.receiver.time.minute,
                 received_gnss.receiver.time.sec);
          if (received_gnss.receiver.pos_dataexist)
          {
            /* Detect position. */
            //ioctl(gnss_fd, CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA, 0);

            printf("## POSITION: LAT %f, LNG %f \n", current_gnss.latitude, current_gnss.longitude);
            send_aud_seq(gnss_jingle, GNSS_JINGLE_LEN);
          }
          else
          {
            //printf("## NO POSITION: sig level %f\n", received_gnss.sv->siglevel);
            //printf("## No Positioning Data\n");
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
  return NULL;
}

bool send_gnss_req(struct gnss_req req)
{
  mqd_t mq = mq_open(GNSS_QUEUE_NAME, O_WRONLY | O_NONBLOCK);
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
  received_gnss.receiver.type = NO_TYPE;
  received_gnss.sv->type = NO_TYPE;
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


int to_gnss( struct cxd56_gnss_positiondata_s* pos_data, struct gnss_data *gnss_data)
{
    gnss_data->date = pos_data->receiver.date;
    gnss_data->time = pos_data->receiver.time;
    gnss_data->latitude = pos_data->receiver.latitude;
    gnss_data->longitude = pos_data->receiver.longitude;
    gnss_data->type = pos_data->sv->type;
    gnss_data->svid = pos_data->sv->svid;
    gnss_data->data_exists = pos_data->receiver.pos_dataexist;
    gnss_data->stat = pos_data->sv->stat;
    gnss_data->siglevel = pos_data->sv->siglevel;
    return OK;
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
