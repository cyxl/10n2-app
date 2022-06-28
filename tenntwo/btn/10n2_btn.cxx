
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
#include <arch/chip/pin.h>
#include <sys/stat.h>
#include <10n2_btn.h>
#include <10n2_aud.h>

static bool btn_running = true;

static pthread_t btn_th_consumer;

void *_btn_q_read(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */

    struct timespec del_sleep
    {
        0, (int)(1 * 1e6)
    };
    while (btn_running)
    {
        int val = board_gpio_read(PIN_PWM0);

        if (val == 0) // on
        {
            send_aud_seq(btn_on_jingle, BTN_ON_JINGLE_LEN);
        }
        printf("%d\n", val);
        {
            nanosleep(&del_sleep, NULL);
        }
    }
    return NULL;
}
static int btn_handler(int irq, FAR void *context, FAR void *arg)
{
    /* Interrupt handler */
    printf("pushed!\n");
}

bool btn_init(void)
{

    printf("btn init\n");
    int ret = board_gpio_config(PIN_PWM0, 0, true, false, PIN_PULLUP);
    if (ret != OK)
    {
        printf("unable to configure pin!\n");
    }
    else
    {
        printf("we good\n");
    }
    btn_running = true;
    board_gpio_intconfig(PIN_PWM0, INT_HIGH_LEVEL, false, btn_handler);
    pthread_create(&btn_th_consumer, NULL, &_btn_q_read, NULL);
    //  board_gpio_int(PIN_PWM0, true);
    return true;
}

bool btn_teardown(void)
{
    printf("btn teardown\n");
    btn_running = false;
    board_gpio_int(PIN_PWM0, false);
    pthread_join(btn_th_consumer, NULL);
    return true;
}
