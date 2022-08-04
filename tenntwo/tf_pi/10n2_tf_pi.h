
#ifndef TENN2_TF_PI_H
#define TENN2_TF_PI_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CELL_IDX 0
#define HANDS_IDX 1
#define NONE_IDX 2
#define NUM_CLASSES 3
#define BAD_IDX -1 
#define UNKNOWN_IDX -42

#define INF_CONF .6

#include <stdint.h>
    struct tf_req
    {
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