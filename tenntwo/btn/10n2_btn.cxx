
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

#define BTN_LONG_HOLD 150
#define BTN_REALLY_SHORT_HOLD 5
#define BTN_SHORT_HOLD 50
#define BTN_DOWN 0
#define BTN_UP 1

static bool btn_running = true;

static pthread_t btn_th_consumer;

uint8_t current_menu = 0;
uint8_t current_submenu = 0;

void play_menu_jingle()
{
    if (current_menu == top)
    {
        send_aud_seq(btn_menu_1_j, BTN_MENU_1_J_LEN);
    }
    else if (current_menu == pos)
    {
        send_aud_seq(btn_menu_2_j, BTN_MENU_2_J_LEN);
    }
    else if (current_menu == img)
    {
        send_aud_seq(btn_menu_3_j, BTN_MENU_3_J_LEN);
    }
    else if (current_menu == inf)
    {
        send_aud_seq(btn_menu_4_j, BTN_MENU_4_J_LEN);
    }

    if ((int)current_submenu == 0)
        send_aud_seq(btn_submenu_1_j, BTN_SUBMENU_1_J_LEN);
    else if ((int)current_submenu == 1)
        send_aud_seq(btn_submenu_2_j, BTN_SUBMENU_2_J_LEN);
    else if ((int)current_submenu == 2)
        send_aud_seq(btn_submenu_3_j, BTN_SUBMENU_3_J_LEN);
    else if ((int)current_submenu == 3)
        send_aud_seq(btn_submenu_4_j, BTN_SUBMENU_4_J_LEN);
    else if ((int)current_submenu == 4)
        send_aud_seq(btn_submenu_5_j, BTN_SUBMENU_5_J_LEN);
    else
        send_aud_seq(btn_err_j, BTN_ERR_J_LEN);
}

void toggle_menu()
{
    current_menu = (current_menu + 1) % num_menu;
    current_submenu = 0;
    play_menu_jingle();
}

void toggle_submenu()
{
    if (current_menu == top)
    {
        current_submenu = (current_submenu + 1) % num_top_menu;
    }
    else if (current_menu == pos)
    {
        current_submenu = (current_submenu + 1) % num_pos_menu;
    }
    else if (current_menu == img)
    {
        current_submenu = (current_submenu + 1) % num_img_menu;
    }
    else if (current_menu == inf)
    {
        current_submenu = (current_submenu + 1) % num_inf_menu;
    }
    play_menu_jingle();
}

void *_btn_q_read(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */

    uint32_t hold_cnt = 0;
    int last_val = 0;

    struct timespec del_sleep
    {
        0, (int)(1 * 1e6)
    };

    bool menu_selecting = false;
    while (btn_running)
    {
        int val = board_gpio_read(PIN_PWM0);

        if (val == BTN_DOWN && val == last_val)
        {
            hold_cnt++;
            if (hold_cnt >= BTN_LONG_HOLD)
            {
                // menu toggle
                printf("menu!\n");
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
                //noop
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
        nanosleep(&del_sleep, NULL);
        last_val = val;
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
