/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <arch/board/board.h>
#include <arch/chip/audio.h>
#include <10n2_aud.h>

bool aud_init(void)
{
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


  return true;
}

bool aud_teardown(void)
{
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

  return true;
}

bool aud_beep(bool en , int16_t vol , uint16_t freq )
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
