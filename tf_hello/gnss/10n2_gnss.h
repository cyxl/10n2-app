#ifndef GNSS_H
#define GNSS_H



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
#endif