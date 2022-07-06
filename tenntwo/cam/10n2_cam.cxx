
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
#include <arch/chip/audio.h>

#include <10n2_cam.h>
#include <tnt_image_provider.h>
#include <camera_fileutil.h>

static bool cam_running = true;
#define CAM_QUEUE_NAME "/cam_queue" /* Queue name. */
#define CAM_QUEUE_PERMS ((int)(0644))
#define CAM_QUEUE_MAXMSG 16               /* Maximum number of messages. */
#define CAM_QUEUE_MSGSIZE sizeof(cam_req) /* Length of message. */
#define CAM_QUEUE_ATTR_INITIALIZER ((struct mq_attr){CAM_QUEUE_MAXMSG, CAM_QUEUE_MSGSIZE, 0, 0})

#define CAM_QUEUE_POLL ((struct timespec){0, 100000000})

#define IMG_SAVE_DIR "/mnt/sd0/img"
static struct mq_attr cam_attr_mq = CAM_QUEUE_ATTR_INITIALIZER;
static pthread_t cam_th_consumer;

void *_cam_q_read(void *args)
{
    
    (void)args; /* Suppress -Wunused-parameter warning. */
    /* Initialize the queue attributes */

    /* Create the message queue. The queue reader is NONBLOCK. */
    mqd_t r_mq = mq_open(CAM_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, CAM_QUEUE_PERMS, &cam_attr_mq);
    futil_initialize();

    if (r_mq < 0)
    {
        fprintf(stderr, "[CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
        return NULL;
    }

    printf("[CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

    unsigned int prio;
    ssize_t bytes_read;
    char buffer[CAM_QUEUE_MSGSIZE];
    char namebuf[128];
    struct timespec poll_sleep;
    // TODO make size configurable

    while (cam_running)
    {
        memset(buffer, 0x00, sizeof(buffer));
        bytes_read = mq_receive(r_mq, buffer, CAM_QUEUE_MSGSIZE, &prio);
        cam_req *r = (cam_req *)buffer;
        if (bytes_read >= 0)
        {
            unsigned curr_time = (unsigned)time(NULL);
           // uint32_t bufsize = r->width * r->height * (r->color?2:1);
            uint16_t width = (r->clip_x1-r->clip_x0); 
            uint16_t height = (r->clip_y1 - r->clip_y0);
            uint32_t bufsize = width * height * (r->color?2:1);
            unsigned char *buf = (unsigned char *)memalign(32, bufsize);
            for (int i = 0; i < r->num; i++)
            {
                bzero(namebuf, 128);
                printf("getting image \n");
                getimage(buf,r->clip_x0,r->clip_y0,r->clip_x1,r->clip_y1,r->color);
                printf("init futil \n");
                printf("writing image\n");
                snprintf(namebuf, 128, "%s/%s/tnt-%i-%i-%ix%i.%s", IMG_SAVE_DIR,r->dir, curr_time, i,width,height, r->color?"yuv":"data");
                if (futil_writeimage(buf, bufsize, namebuf) < 0)
                {
                    printf("ERROR creating image \n");
                }
                struct timespec aud_sleep
                {
                    r->delay / 1000, (r->delay % 1000) * 1e6
                };
                nanosleep(&aud_sleep, NULL);
            }
            free(buf);
        }
        else
        {
            poll_sleep = CAM_QUEUE_POLL;
            nanosleep(&poll_sleep, NULL);
        }

        fflush(stdout);
    }
    printf("cam cleaning mq\n");
    mq_close(r_mq);
    // mq_unlink(CAM_QUEUE_NAME);
    return NULL;
}

bool send_cam_req(struct cam_req req)
{
    mqd_t mq = mq_open(CAM_QUEUE_NAME, O_WRONLY);
    if (mq < 0)
    {
        fprintf(stderr, "[sender]: Error, cannot open the queue: %s.\n", strerror(errno));
        return false;
    }
    if (mq_send(mq, (char *)&req, CAM_QUEUE_MSGSIZE, 1) < 0)
    {
        fprintf(stderr, "[sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
    }

    mq_close(mq);
    return true;
}

bool cam_init(void)
{
    printf("bws cam init\n");
    cam_running = true;
    pthread_create(&cam_th_consumer, NULL, &_cam_q_read, NULL);
    return true;
}

bool cam_teardown(void)
{
    printf("cam teardown\n");
    cam_running = false;
    pthread_join(cam_th_consumer, NULL);
    return true;
}
