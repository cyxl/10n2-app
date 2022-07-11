
#ifndef TENN2_TF_PI_H
#define TENN2_TF_PI_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
struct tf_req{
    uint32_t num;
    int16_t delay;
};

bool tf_pi_init(void);
bool tf_pi_teardown(void);

bool send_tf_req(struct tf_req req);
#ifdef __cplusplus
}
#endif
#endif