

#ifndef TENN2_AUD_H
#define TENN2_AUD_H

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif

bool aud_init(void);

bool aud_teardown(void);

bool aud_beep(bool en, int16_t vol , uint16_t freq );

#ifdef __cplusplus
}
#endif

#endif  
