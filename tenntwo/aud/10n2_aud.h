

#ifndef TENN2_AUD_H
#define TENN2_AUD_H

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C"
{
#endif

#define OFF_FREQ 65535
  struct aud_data
  {
    bool en;
    int16_t vol;
    uint16_t freq;
    uint32_t dur; // milli
  };

#define DEF_VOLUME -20
#define JINGLE_MAXLEN 10

  enum aud_jingle_type
  {
    startup_j = 0,
    shutdown_j,
    gnss,
    btn_on,
    btn_menu_1,
    btn_menu_2,
    btn_menu_3,
    btn_menu_4,
    btn_submenu_1,
    btn_submenu_2,
    btn_submenu_3,
    btn_submenu_4,
    btn_submenu_5,
    btn_err,
    tf_bad,
    tf_cell,
    tf_none,
    tf_hands,
    tf_nohands,
    cam_capture,
    imu_turn,
    imu_accel,
    imu_decel,
    imu_pothole,
    short_pause,
    num_j
  };

  extern struct aud_data all_j[num_j][JINGLE_MAXLEN];

  bool aud_init(void);

  bool aud_teardown(void);

  bool aud_beep(bool en, int16_t vol, uint16_t freq);

  bool send_aud_seq(enum aud_jingle_type j);

#ifdef __cplusplus
}
#endif

#endif
