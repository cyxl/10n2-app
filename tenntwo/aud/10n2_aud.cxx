/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/types.h>

#include <arch/board/board.h>
#include <arch/chip/audio.h>
#include <nuttx/arch.h>
#include <10n2_aud.h>
#include <fcntl.h>

static bool running = true;
#define AUD_QUEUE_NAME "/aud_queue2" /* Queue name. */
#define AUD_QUEUE_PERMS ((int)(0644))
#define AUD_QUEUE_MAXMSG 64                       /* Maximum number of messages. */
#define AUD_QUEUE_MSGSIZE sizeof(aud_jingle_type) /* Length of message. */
#define AUD_QUEUE_ATTR_INITIALIZER ((struct mq_attr){AUD_QUEUE_MAXMSG, AUD_QUEUE_MSGSIZE, 0, 0})

static struct mq_attr attr_mq = AUD_QUEUE_ATTR_INITIALIZER;
static pthread_t th_consumer;

mqd_t mq = 0;

struct aud_data all_j[num_j][10] = {

    {
        // startup_j
        {1, DEF_VOLUME, 400, 10},
        {1, DEF_VOLUME, 500, 10},
        {1, DEF_VOLUME, 600, 10},
        {1, DEF_VOLUME, 600, 10},
        {1, DEF_VOLUME, 300, 10},
        {1, DEF_VOLUME, 300, 10},
        {1, DEF_VOLUME, 600, 10},
        {1, DEF_VOLUME, 500, 10},
        {1, DEF_VOLUME, 400, 10},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // shutdown_j
        {1, DEF_VOLUME, 300, 100},
        {1, DEF_VOLUME, 600, 100},
        {1, DEF_VOLUME, 600, 100},
        {1, DEF_VOLUME, 500, 100},
        {1, DEF_VOLUME, 400, 100},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // gnss
        {1, DEF_VOLUME, 1900, 10},
        {0, 255, 0, 0}, // off
        {1, DEF_VOLUME, 2000, 20},
        {0, 255, 0, 0}, // off
        {1, DEF_VOLUME, 1900, 10},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_on
        {1, DEF_VOLUME, 1400, 10},
        {0, 255, 0, 0}, // off
        {1, DEF_VOLUME, 1600, 0},
        {0, 255, 0, 10}, // off
        {1, DEF_VOLUME, 1400, 0},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_menu_1
        {1, DEF_VOLUME, 700, 150},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_menu_2
        {1, DEF_VOLUME, 700, 150},
        {0, 255, 0, 40}, // off
        {1, DEF_VOLUME, 700, 150},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_menu_3
        {1, DEF_VOLUME, 700, 150},
        {0, 255, 0, 40}, // off
        {1, DEF_VOLUME, 700, 150},
        {0, 255, 0, 40}, // off
        {1, DEF_VOLUME, 700, 150},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_menu_4
        {1, DEF_VOLUME, 700, 150},
        {0, 255, 0, 40}, // off
        {1, DEF_VOLUME, 700, 150},
        {0, 255, 0, 40}, // off
        {1, DEF_VOLUME, 700, 150},
        {0, 255, 0, 0}, // off
        {1, DEF_VOLUME, 700, 150},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_submenu_1
        {1, DEF_VOLUME, 900, 100},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_submenu_2
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_submenu_3
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_submenu_4
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_submenu_5
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, 0, 20}, // off
        {1, DEF_VOLUME, 900, 100},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // btn_err
        {1, DEF_VOLUME, 300, 400},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // tf_bad
        {1, DEF_VOLUME, 100, 120},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // tf_cell
        {1, VIOLATIN_VOLUME, 100, 20},
        {0, 255, 0, 1}, // off
        {1, VIOLATIN_VOLUME, 100, 20},
        {0, 255, 0, 1}, // off
        {1, VIOLATIN_VOLUME, 100, 20},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // tf_none
        {1, VIOLATIN_VOLUME, 100, 60},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // tf_hands
        {1, DEF_VOLUME, 1800, 40},
        {0, 255, 0, 0}, // off
        {1, DEF_VOLUME, 1800, 40},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // tf_nohands
        {1, DEF_VOLUME, 1500, 50},
        {1, DEF_VOLUME, 300, 50},
        {1, DEF_VOLUME, 1500, 50},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // cam_capture
        {1, DEF_VOLUME, 1800, 10},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // imu_turn
        {1, DEF_VOLUME, 1800, 50},
        {0, 255, 0, 0},
        {1, DEF_VOLUME, 1800, 50},
        {0, 255, 0, 0},
        {1, DEF_VOLUME, 300, 50},
        {0, 255, 0, 0},
        {1, DEF_VOLUME, 300, 50},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // imu_decel
        {1, DEF_VOLUME, 1500, 50},
        {1, DEF_VOLUME, 1200, 50},
        {1, DEF_VOLUME, 900, 50},
        {1, DEF_VOLUME, 600, 50},
        {1, DEF_VOLUME, 300, 50},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // imu_accel
        {1, DEF_VOLUME, 300, 50},
        {1, DEF_VOLUME, 600, 50},
        {1, DEF_VOLUME, 900, 50},
        {1, DEF_VOLUME, 1200, 50},
        {1, DEF_VOLUME, 1500, 50},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // imu_pothole
        {1, DEF_VOLUME, 300, 5},
        {0, 255, 0, 1},
        {1, DEF_VOLUME, 300, 5},
        {0, 255, 0, 1},
        {1, DEF_VOLUME, 300, 5},
        {0, 255, OFF_FREQ, 0}, // done
    },
    {
        // short_pause 
        {0, 255, 0, 10},
        {0, 255, OFF_FREQ, 0}, // done
    },

};

void *_q_read(void *args)
{
  (void)args; /* Suppress -Wunused-parameter warning. */
  /* Initialize the queue attributes */
  // int cpu = up_cpu_index();
  // printf("aud CPU %d\n", cpu);

  sigset_t mask;
  sigemptyset(&mask);
  // TOOD 18
  sigaddset(&mask, 18);
  int ret = sigprocmask(SIG_BLOCK, &mask, NULL);
  if (ret != OK)
  {
    printf("TF ERROR sigprocmask failed. %d\n", ret);
  }
  /* Create the message queue. The queue reader is NONBLOCK. */
  mqd_t r_mq = mq_open(AUD_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, AUD_QUEUE_PERMS, &attr_mq);

  if (r_mq < 0)
  {
    fprintf(stderr, "[CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
    return NULL;
  }

  printf("[CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

  unsigned int prio;
  ssize_t bytes_read;
  char buffer[AUD_QUEUE_MSGSIZE];
  struct timespec poll_sleep;

  while (running)
  {
    memset(buffer, 0x00, sizeof(buffer));
    bytes_read = mq_receive(r_mq, buffer, AUD_QUEUE_MSGSIZE, &prio);
    aud_jingle_type *r = (aud_jingle_type *)buffer;
    if (bytes_read >= 0)
    {
      if (*r >= num_j)
      {
        printf("ERROR - Jingle not recognized!\n");
        continue;
      }

      aud_data *d = all_j[*r];

      for (int j_idx = 0; j_idx <= JINGLE_MAXLEN; j_idx++)
      {

        if (d[j_idx].freq == OFF_FREQ)
        {
          aud_beep(0, 255, 0);
          continue;
        }
        int rc = aud_beep(d[j_idx].en, d[j_idx].vol, d[j_idx].freq);
        if (rc < 0)
          printf("aud beep failed %i\n", rc);
        usleep(d[j_idx].dur * 1e3);
        // off
        rc = aud_beep(0, 255, 0);
        if (rc < 0)
          printf("aud beep failed %i\n", rc);
      }
    }
    else
    {
      usleep(10 * 1e3);
    }
  }
  printf("aud cleaning mq\n");
  mq_close(r_mq);
  // mq_unlink(QUEUE_NAME);
  return NULL;
}

bool send_aud_seq(aud_jingle_type j)
{
  if (mq == 0)
  {
    mq = mq_open(AUD_QUEUE_NAME, O_WRONLY | O_NONBLOCK);
    if (mq < 0)
    {
      fprintf(stderr, "[aud sender]: Error, cannot open the queue: %s.\n", strerror(errno));
      return false;
    }
  }
  if (mq_send(mq, (char *)&j, sizeof(aud_jingle_type), 1) < 0)
  {
    fprintf(stderr, "[aud sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
  }

  return true;
}

bool aud_init(void)
{
  running = true;

  printf("init audio\n");

  /* Enable I2S pin. */
  cxd56_audio_en_i2s_io();

  /* Enable speaker output. */
  cxd56_audio_set_spout(true);

  /* Power on Audio driver */
  if (cxd56_audio_poweron() != CXD56_AUDIO_ECODE_OK)
  {
    return false;
  }

  /* Enable BaseBand driver output */
  if (cxd56_audio_en_output() != CXD56_AUDIO_ECODE_OK)
  {
    return false;
  }

  if (board_external_amp_mute_control(false) != OK)
  {
    return false;
  }

  cpu_set_t cpuset = 1 << 2;
  pthread_create(&th_consumer, NULL, &_q_read, NULL);
  int rc;
  rc = pthread_setaffinity_np(th_consumer, sizeof(cpu_set_t), &cpuset);
  if (rc != 0)
  {
    printf("Unable set CPU affinity : %d", rc);
  }

  // off
  aud_beep(0, 255, 0);
  printf("done init audio\n");
  return true;
}

bool aud_teardown(void)
{
  printf("tearing down audio\n");
  /* Disable speaker output. */
  cxd56_audio_set_spout(false);

  /* Disable I2S pin. */

  cxd56_audio_dis_i2s_io();

  /* Power off Audio driver */

  if (cxd56_audio_dis_output() != CXD56_AUDIO_ECODE_OK)
  {
    return false;
  }

  /* Disable BaseBand driver output */

  if (cxd56_audio_poweroff() != CXD56_AUDIO_ECODE_OK)
  {
    return false;
  }

  running = false;
  pthread_join(th_consumer, NULL);
  return true;
}

bool aud_beep(bool en, int16_t vol, uint16_t freq)
{
  if (!en)
  {
    /* Stop beep */

    if (cxd56_audio_stop_beep() != CXD56_AUDIO_ECODE_OK)
    {
      return false;
    }
  }

  if (0 != freq)
  {
    /* Set beep frequency parameter */

    if (cxd56_audio_set_beep_freq(freq) != CXD56_AUDIO_ECODE_OK)
    {
      return false;
    }
  }

  if (255 != vol)
  {
    /* Set beep volume parameter */

    if (cxd56_audio_set_beep_vol(vol) != CXD56_AUDIO_ECODE_OK)
    {
      return false;
    }
  }

  if (en)
  {
    /* Play beep */

    if (cxd56_audio_play_beep() != CXD56_AUDIO_ECODE_OK)
    {
      return false;
    }
  }

  return true;
}
