

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
#include <debug_log_callback.h>
#include <main_functions.h>
#include <10n2_tf_pi.h>

static bool tf_pi_running = true;
#define TF_PI_QUEUE_NAME "/tf_pi_queue" /* Queue name. */
#define TF_PI_QUEUE_PERMS ((int)(0644))
#define TF_PI_QUEUE_MAXMSG 16                 /* Maximum number of messages. */
#define TF_PI_QUEUE_MSGSIZE sizeof(tf_pi_req) /* Length of message. */
#define TF_PI_QUEUE_ATTR_INITIALIZER ((struct mq_attr){TF_PI_QUEUE_MAXMSG, TF_PI_QUEUE_MSGSIZE, 0, 0})

#define TF_PI_QUEUE_POLL ((struct timespec){0, 100000000})

#define TF_PI_SAVE_DIR "/mnt/sd0"
static struct mq_attr tf_pi_attr_mq = TF_PI_QUEUE_ATTR_INITIALIZER;
static pthread_t tf_pi_th_consumer;

static void test_log_printf(const char *s)
{
    printf("bws %s ", s);
}

void *_tf_pi_q_read(void *args)
{

    (void)args; /* Suppress -Wunused-parameter warning. */
    /* Initialize the queue attributes */

    /* Create the message queue. The queue reader is NONBLOCK. */
    mqd_t r_mq = mq_open(TF_PI_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, TF_PI_QUEUE_PERMS, &tf_pi_attr_mq);
    char tf_pi_buf[sizeof(tf_pi_req)];

    if (r_mq < 0)
    {
        fprintf(stderr, "[CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
        return NULL;
    }

    printf("[CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

    unsigned int prio;
    ssize_t bytes_read;
    char buffer[TF_PI_QUEUE_MSGSIZE];
    struct timespec poll_sleep;
    // TODO make size configurable

    while (tf_pi_running)
    {
        bytes_read = mq_receive(r_mq, buffer, TF_PI_QUEUE_MSGSIZE, &prio);
        tf_pi_req *r = (tf_pi_req *)buffer;
        if (bytes_read >= 0)
        {
            for (int i = 0; i < r->num; i++)
            {
                loop();
                struct timespec del_sleep
                {
                    r->delay / 1000, (r->delay % 1000) * 1e6
                };
                nanosleep(&del_sleep, NULL);
            }
        }
        else
        {
            poll_sleep = TF_PI_QUEUE_POLL;
            nanosleep(&poll_sleep, NULL);
        }

        fflush(stdout);
    }
    printf("TF PI cleaning mq\n");
    mq_close(r_mq);
    // mq_unlink(TF_PI_QUEUE_NAME);
    return NULL;
}

bool send_tf_pi_req(struct tf_pi_req req)
{
    mqd_t mq = mq_open(TF_PI_QUEUE_NAME, O_WRONLY);
    if (mq < 0)
    {
        fprintf(stderr, "[sender]: Error, cannot open the queue: %s.\n", strerror(errno));
        return false;
    }
    if (mq_send(mq, (char *)&req, TF_PI_QUEUE_MSGSIZE, 1) < 0)
    {
        fprintf(stderr, "[sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
    }

    mq_close(mq);
    return true;
}

bool tf_pi_init(void)
{
    printf("tf pi init\n");
    tf_pi_running = true;
    RegisterDebugLogCallback(test_log_printf);
    setup();
    pthread_create(&tf_pi_th_consumer, NULL, &_tf_pi_q_read, NULL);

    return true;
}

bool tf_pi_teardown(void)
{
    printf("tf pi teardown\n");
    tf_pi_running = false;
    pthread_join(tf_pi_th_consumer, NULL);
    return true;
}
