
/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <arch/board/board.h>
#include <nuttx/clock.h>
#include <10n2_rec.h>
#include <10n2_imu.h>
#include <10n2_aud.h>
#include <10n2_btn.h>
#include <10n2_cam.h>
#include <10n2_gnss.h>
#include <10n2_dp.h>
#include <10n2_tf_pi.h>

static bool rec_running = true;
#define REC_QUEUE_NAME "/rec_queue" /* Queue name. */
#define REC_QUEUE_PERMS ((int)(0644))
#define REC_QUEUE_MAXMSG 16               /* Maximum number of messages. */
#define REC_QUEUE_MSGSIZE sizeof(rec_req) /* Length of message. */
#define REC_QUEUE_ATTR_INITIALIZER ((struct mq_attr){REC_QUEUE_MAXMSG, REC_QUEUE_MSGSIZE, 0, 0})

#define REC_QUEUE_POLL ((struct timespec){0, 100000000})

static pthread_t rec_th;
static struct mq_attr rec_attr_mq = REC_QUEUE_ATTR_INITIALIZER;

#define POS_SAVE_DIR "/mnt/sd0/pos"

FILE *pos_pf = NULL;
void close_pos_fd()
{
    if (pos_pf != NULL)
    {
        if (fclose(pos_pf) != 0)
        {
            printf("Unable to close pos file! :%s\n", strerror(errno));
        }
        else
        {
            printf("success!  closed pos output file\n");
        }
        pos_pf = NULL;
    }
}

void open_pos_fd(uint8_t cnt)
{
    unsigned curr_time = clock();
    char namebuf[128];
    snprintf(namebuf, 128, "%s/imu-data-%i-%i.%s", POS_SAVE_DIR, cnt, curr_time, "csv");
    printf("opening :%s\n", namebuf);
    pos_pf = fopen(namebuf, "wb+");
    if (pos_pf == NULL)
    {
        printf("Unable to open pos! :%s\n", strerror(errno));
    }
    else
    {
        printf("success!  opened pos output file\n");
    }
    fprintf(pos_pf, "t,slopex,slopey,slopez,inf,conf,acx,acy,acz,gyx,gyy,gyz,y,M,d,h,m,s,us,t,lat,lon\n");
}

void *_rec_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */
    mqd_t r_mq = mq_open(REC_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, REC_QUEUE_PERMS, &rec_attr_mq);
    if (r_mq < 0)
    {
        fprintf(stderr, "[rec CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
        return NULL;
    }
    printf("[rec CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);
    unsigned int prio;
    ssize_t bytes_read;
    char buffer[REC_QUEUE_MSGSIZE];
    struct timespec poll_sleep;

    while (rec_running)
    {
        bytes_read = mq_receive(r_mq, buffer, REC_QUEUE_MSGSIZE, &prio);
        rec_req *r = (rec_req *)buffer;
        if (bytes_read >= 0)
        {
            struct timespec rec_sleep
            {
                r->delay / 1000, (r->delay % 1000) * 1e6
            };
            if (r->type == rec_open)
            {
                open_pos_fd(r->f_id);
            }
            else if (r->type == rec_close)
            {
                close_pos_fd();
            }
            else
            {
                for (int i = 0; i < r->num; i++)
                {
                    printf("recording!\n");
                    unsigned curr_time = clock();
                    fprintf(pos_pf, "%i,%f,%f,%f,%i,%f,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%lf,%lf\n",
                            curr_time,
                            current_x_slope,
                            current_y_slope,
                            current_z_slope,
                            current_inf,
                            current_conf,
                            current_pmu.ac_x,
                            current_pmu.ac_y,
                            current_pmu.ac_z,
                            current_pmu.gy_x,
                            current_pmu.gy_y,
                            current_pmu.gy_z,
                            current_gnss.date.year,
                            current_gnss.date.month,
                            current_gnss.date.day,
                            current_gnss.time.hour,
                            current_gnss.time.minute,
                            current_gnss.time.sec,
                            current_gnss.time.usec,
                            current_gnss.type,
                            current_gnss.latitude,
                            current_gnss.longitude);
                    nanosleep(&rec_sleep, NULL);
                }
            }
        }
        else
        {
            poll_sleep = REC_QUEUE_POLL;
            nanosleep(&poll_sleep, NULL);
        }
    }
    printf("rec cleaning mq\n");
    mq_close(r_mq);
    return NULL;
}

bool send_rec_req(struct rec_req req)
{
    mqd_t mq = mq_open(REC_QUEUE_NAME, O_WRONLY);
    if (mq < 0)
    {
        fprintf(stderr, "[rec sender]: Error, cannot open the queue: %s.\n", strerror(errno));
        return false;
    }
    if (mq_send(mq, (char *)&req, REC_QUEUE_MSGSIZE, 1) < 0)
    {
        fprintf(stderr, "[rec sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
    }

    mq_close(mq);
    return true;
}
bool rec_init(void)
{
    printf("menu handler init\n");
    rec_running = true;
    pthread_create(&rec_th, NULL, &_rec_run, NULL);

    return true;
}

bool rec_teardown(void)
{
    printf("menu handler teardown\n");
    rec_running = false;
    pthread_join(rec_th, NULL);
    return true;
}
