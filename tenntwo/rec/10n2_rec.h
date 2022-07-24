
#ifndef TENN2_REC_H
#define TENN2_REC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    rec_open=0,
    rec_close,
    rec_write,
}rec_act;
typedef enum {
    rec_verbose = 0,
    rec_terse,
}rec_type;
struct rec_req
{
    int8_t num;
    int16_t delay;
    rec_act act;
    rec_type type;
    uint8_t f_id;
};

bool send_rec_req(struct rec_req req);

bool rec_init(void);
bool rec_teardown(void);

#ifdef __cplusplus
}
#endif
#endif