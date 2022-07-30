
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
#include <nuttx/arch.h>
#include <sys/stat.h>
#include <10n2_btn.h>
#include <10n2_aud.h>

#define BTN_LONG_HOLD 24
#define BTN_REALLY_SHORT_HOLD 1
#define BTN_SHORT_HOLD 10
#define BTN_DOWN 0
#define BTN_UP 1

static bool btn_running = true;

static pthread_t btn_th_consumer;

uint8_t current_menu = 0;
uint8_t current_submenu = 0;

pthread_mutex_t btn_mutex;

void play_menu_jingle()
{
    if (current_menu == top)
    {
        send_aud_seq(btn_menu_1);
    }
    else if (current_menu == inf)
    {
        send_aud_seq(btn_menu_2);
    }
    else if (current_menu == train)
    {
        send_aud_seq(btn_menu_3);
    }

    send_aud_seq(short_pause);

    if ((int)current_submenu == 0)
        send_aud_seq(btn_submenu_1);
    else if ((int)current_submenu == 1)
        send_aud_seq(btn_submenu_2);
    else if ((int)current_submenu == 2)
        send_aud_seq(btn_submenu_3);
    else if ((int)current_submenu == 3)
        send_aud_seq(btn_submenu_4);
    else if ((int)current_submenu == 4)
        send_aud_seq(btn_submenu_5);
    else
        send_aud_seq(btn_err);
}

void toggle_menu()
{
    btn_wait();
    current_menu = (current_menu + 1) % num_menu;
    current_submenu = 0;
    btn_release();
    play_menu_jingle();
}

void toggle_submenu()
{
    btn_wait();
    if (current_menu == top)
    {
        current_submenu = (current_submenu + 1) % num_top_menu;
    }
    else if (current_menu == inf)
    {
        current_submenu = (current_submenu + 1) % num_inf_menu;
    }
    else if (current_menu == train)
    {
        current_submenu = (current_submenu + 1) % num_train_menu;
    }
    btn_release();
    play_menu_jingle();
}

void *_btn_q_read(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */

    //int cpu = up_cpu_index();
    //printf("Button CPU %d\n", cpu);

    uint32_t hold_cnt = 0;
    int last_val = 0;

    bool menu_selecting = false;
    while (btn_running)
    {
        usleep(50 * 1e3);
        int val = board_gpio_read(PIN_PWM0);

        if (val == BTN_DOWN && val == last_val)
        {
            hold_cnt++;
            if (hold_cnt >= BTN_LONG_HOLD)
            {
                // menu toggle
                printf("menu!\n %d", hold_cnt);
                toggle_menu();
                menu_selecting = true;
                hold_cnt = 0;
            }
        }
        if (val == BTN_UP && last_val == BTN_DOWN)
        {
            printf("cnt %i\n", hold_cnt);
            if (menu_selecting)
            {
                // noop
            }
            else if (hold_cnt <= BTN_REALLY_SHORT_HOLD)
            {
                play_menu_jingle();
            }
            else if (hold_cnt <= BTN_SHORT_HOLD)
            {
                // sub menu toggle
                printf("submenu!\n");
                toggle_submenu();
            }
            menu_selecting = false;
            hold_cnt = 0;
        }
        last_val = val;
    }
    return NULL;
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
        printf("gio configured\n");
    }
    btn_running = true;
    cpu_set_t cpuset = 1 << 2;
    pthread_create(&btn_th_consumer, NULL, &_btn_q_read, NULL);
    int rc;
    //rc = pthread_setaffinity_np(btn_th_consumer, sizeof(cpu_set_t), &cpuset);

    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }

    if (0 != (errno = pthread_mutex_init(&btn_mutex, NULL)))
    {
        perror("pthread_mutex_init() failed");
        return EXIT_FAILURE;
    }
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

bool btn_wait(void)
{
    return 0 == pthread_mutex_lock(&btn_mutex);
}
bool btn_release(void)
{
    return 0 == pthread_mutex_unlock(&btn_mutex);
}
