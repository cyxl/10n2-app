
#ifndef TENN2_REC_H
#define TENN2_REC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    rec_open=0,
    rec_close,
    rec_write
}rec_act;
struct rec_req
{
    int8_t num;
    int16_t delay;
    rec_act type;
    uint8_t f_id;
};

bool send_rec_req(struct rec_req req);

bool rec_init(void);
bool rec_teardown(void);

#ifdef __cplusplus
}
#endif
#endif