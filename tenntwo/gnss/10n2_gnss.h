#ifndef GNSS_H
#define GNSS_H

#define NO_TYPE 42

#ifdef __cplusplus
extern "C"
{
#endif

#include <arch/chip/gnss_type.h>

  struct cxd56_gnss_dms_s
  {
    int8_t sign;
    uint8_t degree;
    uint8_t minute;
    uint32_t frac;
  };

  struct gnss_data
  {
    struct cxd56_gnss_date_s date;
    struct cxd56_gnss_time_s time;
    double latitude;
    double longitude;
    uint16_t type;
    uint8_t svid;
    uint8_t stat;
    bool data_exists;
    float siglevel;
  };
  int to_gnss(struct cxd56_gnss_positiondata_s *pos_data, struct gnss_data *gnss_data);
  int gnss_setparams(int fd);

  struct gnss_req
  {
    uint32_t num;
    int16_t delay;
  };

  bool gnss_init(void);
  bool gnss_teardown(void);

  bool send_gnss_req(struct gnss_req req);

  extern struct gnss_data current_gnss;

#ifdef __cplusplus
}
#endif

#endif