
#ifndef TENN2_TF_PI_H
#define TENN2_TF_PI_H

#ifdef __cplusplus
extern "C" {
#endif

#define TF_UNKNOWN 0
#define TF_HANDS   1
#define TF_NOHANDS 2
#define TF_CELL    3

#include <stdint.h>
struct tf_req{
    uint32_t num;
    int16_t delay;
};

bool tf_pi_init(void);
bool tf_pi_teardown(void);

extern uint8_t current_inf;
extern float current_conf;
extern uint32_t current_time_tf;
bool send_tf_req(struct tf_req req);
#ifdef __cplusplus
}
#endif
#endif