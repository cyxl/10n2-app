#ifndef GNSS_H
#define GNSS_H


#ifdef __cplusplus
extern "C" {
#endif

struct cxd56_gnss_dms_s
{
  int8_t   sign;
  uint8_t  degree;
  uint8_t  minute;
  uint32_t frac;
};

void double_to_dmf(double x, struct cxd56_gnss_dms_s * dmf);
int read_and_print(int fd);
int gnss_setparams(int fd);

struct gnss_req{
    uint32_t num;
    int16_t delay;
};

bool gnss_init(void);
bool gnss_teardown(void);

bool send_gnss_req(struct gnss_req req);

#ifdef __cplusplus
}
#endif

#endif