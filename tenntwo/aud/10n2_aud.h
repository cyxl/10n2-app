

#ifndef TENN2_AUD_H
#define TENN2_AUD_H

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif

struct aud_req
{
  int16_t vol;
  uint16_t freq;
  uint32_t dur; // milli
};

#define STARTUP_JINGLE_LEN 5

static struct aud_req startup_jingle[STARTUP_JINGLE_LEN] = {
  {-40,400,100},
  {-40,500,100},
  {-40,600,100},
  {-40,600,100},
  {-40,300,100},
};

bool aud_init(void);

bool aud_teardown(void);

bool aud_beep(bool en, int16_t vol , uint16_t freq );

bool send_aud_beep(int16_t vol, uint16_t freq,uint32_t dur);

bool send_aud_seq(struct aud_req* req,uint8_t len);

#ifdef __cplusplus
}
#endif

#endif  
