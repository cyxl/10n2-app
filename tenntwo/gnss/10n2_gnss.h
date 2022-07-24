#ifndef GNSS_H
#define GNSS_H

#define NO_TYPE 42

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/chip/gnss_type.h>

struct cxd56_gnss_dms_s
{
  int8_t   sign;
  uint8_t  degree;
  uint8_t  minute;
  uint32_t frac;
};

struct gnss_data{
  struct cxd56_gnss_date_s date;
  struct cxd56_gnss_time_s time;
  double latitude;
  double longitude;
  uint16_t type;
  uint8_t svid;
  uint8_t stat;
  float   siglevel;

};
void double_to_dmf(double x, struct cxd56_gnss_dms_s * dmf);
int read_gnss(int fd,struct gnss_data* d);
int read_and_print(int fd);
int gnss_setparams(int fd);

struct gnss_req{
    uint32_t num;
    int16_t delay;
};

bool gnss_init(void);
bool gnss_teardown(void);

bool send_gnss_req(struct gnss_req req);

extern struct cxd56_gnss_positiondata_s current_gnss;

#ifdef __cplusplus
}
#endif

#endif