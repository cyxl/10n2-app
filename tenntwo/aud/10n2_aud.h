

#ifndef TENN2_AUD_H
#define TENN2_AUD_H

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif

struct aud_req
{
  bool en;
  int16_t vol;
  uint16_t freq;
  uint32_t dur; // milli
};

#define STARTUP_JINGLE_LEN 10
#define DEF_VOLUME -20

static struct aud_req startup_jingle[STARTUP_JINGLE_LEN] = {
  {1,DEF_VOLUME,400,10},
  {1,DEF_VOLUME,500,10},
  {1,DEF_VOLUME,600,10},
  {1,DEF_VOLUME,600,10},
  {1,DEF_VOLUME,300,10},
  {1,DEF_VOLUME,300,10},
  {1,DEF_VOLUME,600,10},
  {1,DEF_VOLUME,600,10},
  {1,DEF_VOLUME,500,10},
  {1,DEF_VOLUME,400,10},
};

#define SHUTDOWN_JINGLE_LEN 5

static struct aud_req shutdown_jingle[SHUTDOWN_JINGLE_LEN] = {
  {1,DEF_VOLUME,300,100},
  {1,DEF_VOLUME,600,100},
  {1,DEF_VOLUME,600,100},
  {1,DEF_VOLUME,500,100},
  {1,DEF_VOLUME,400,100},
};

#define GNSS_JINGLE_LEN 6 
static struct aud_req gnss_jingle[GNSS_JINGLE_LEN] = {
  {1,DEF_VOLUME,1900,10}, {0,255,0,0}, //off
  {1,DEF_VOLUME,2000,20}, {0,255,0,0}, //off
  {1,DEF_VOLUME,1900,10}, {0,255,0,0}, //off
};


#define BTN_ON_JINGLE_LEN 6

static struct aud_req btn_on_jingle[BTN_ON_JINGLE_LEN] = {
  {1,DEF_VOLUME,1400,10},
  {0,255,0,0}, //off
  {1,DEF_VOLUME,1600,0},
  {0,255,0,10}, //off
  {1,DEF_VOLUME,1400,0},
  {0,255,0,10}, //off
};

#define BTN_MENU_1_J_LEN 2
static struct aud_req btn_menu_1_j[BTN_MENU_1_J_LEN] = {
  {1,DEF_VOLUME,700,150}, {0,255,0,0}, //off
};

#define BTN_MENU_2_J_LEN 4
static struct aud_req btn_menu_2_j[BTN_MENU_2_J_LEN] = {
  {1,DEF_VOLUME,700,150}, {0,255,0,40}, //off
  {1,DEF_VOLUME,700,150}, {0,255,0,0}, //off
};

#define BTN_MENU_3_J_LEN 6
static struct aud_req btn_menu_3_j[BTN_MENU_3_J_LEN] = {
  {1,DEF_VOLUME,700,150}, {0,255,0,40}, //off
  {1,DEF_VOLUME,700,150}, {0,255,0,40}, //off
  {1,DEF_VOLUME,700,150}, {0,255,0,0}, //off
};

#define BTN_MENU_4_J_LEN 8 
static struct aud_req btn_menu_4_j[BTN_MENU_4_J_LEN] = {
  {1,DEF_VOLUME,700,150}, {0,255,0,40}, //off
  {1,DEF_VOLUME,700,150}, {0,255,0,40}, //off
  {1,DEF_VOLUME,700,150}, {0,255,0,0}, //off
  {1,DEF_VOLUME,700,150}, {0,255,0,0}, //off
};

#define BTN_SUBMENU_1_J_LEN 2
static struct aud_req btn_submenu_1_j[BTN_SUBMENU_1_J_LEN] = {
  {1,DEF_VOLUME,900,100}, {0,255,0,0}, //off
};

#define BTN_SUBMENU_2_J_LEN 4
static struct aud_req btn_submenu_2_j[BTN_SUBMENU_2_J_LEN] = {
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,0}, //off
};

#define BTN_SUBMENU_3_J_LEN 6
static struct aud_req btn_submenu_3_j[BTN_SUBMENU_3_J_LEN] = {
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,0}, //off
};

#define BTN_SUBMENU_4_J_LEN 8
static struct aud_req btn_submenu_4_j[BTN_SUBMENU_4_J_LEN] = {
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,0}, //off
};
#define BTN_SUBMENU_5_J_LEN 10
static struct aud_req btn_submenu_5_j[BTN_SUBMENU_5_J_LEN] = {
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,20}, //off
  {1,DEF_VOLUME,900,100}, {0,255,0,0}, //off
};
#define BTN_ERR_J_LEN 2
static struct aud_req btn_err_j[BTN_ERR_J_LEN] = {
  {1,DEF_VOLUME,300,400}, {0,255,0,0}, //off
};

#define TF_CELL_J_LEN 2
static struct aud_req tf_cell_j[TF_CELL_J_LEN] = {
  {1,DEF_VOLUME,200,80}, {0,255,0,0}, //off
};
#define TF_HANDS_J_LEN 4
static struct aud_req tf_hands_j[TF_HANDS_J_LEN] = {
  {1,DEF_VOLUME,1800,40}, {0,255,0,0}, //off
  {1,DEF_VOLUME,1800,40}, {0,255,0,0}, //off
};
#define TF_NONE_J_LEN 4
static struct aud_req tf_none_j[TF_NONE_J_LEN] = {
  {1,DEF_VOLUME,500,40}, {0,255,0,0}, //off
  {1,DEF_VOLUME,500,40}, {0,255,0,0}, //off
};

#define CAM_CAPTURE_J_LEN 2
static struct aud_req cam_cap_j[CAM_CAPTURE_J_LEN] = {
  {1,DEF_VOLUME,1800,10}, {0,255,0,0}, //off
};
  
#define SHARP_TURN_J_LEN 8
static struct aud_req sharp_turn_j[SHARP_TURN_J_LEN] = {
  {1,DEF_VOLUME,1800,50},{0,255,0,0},
  {1,DEF_VOLUME,1800,50},{0,255,0,0},
  {1,DEF_VOLUME,300,50},{0,255,0,0},
  {1,DEF_VOLUME,300,50}, {0,255,0,0},
};
  
#define DECELERATION_J_LEN 6
static struct aud_req deceleration_j[DECELERATION_J_LEN] = {
  {1,DEF_VOLUME,1500,50}, 
  {1,DEF_VOLUME,1200,50}, 
  {1,DEF_VOLUME,900,50}, 
  {1,DEF_VOLUME,600,50}, 
  {1,DEF_VOLUME,300,50}, {0,255,0,0},
};

#define ACCELERATION_J_LEN 6
static struct aud_req acceleration_j[ACCELERATION_J_LEN] = {
  {1,DEF_VOLUME,300,50},
  {1,DEF_VOLUME,600,50},
  {1,DEF_VOLUME,900,50},
  {1,DEF_VOLUME,1200,50},
  {1,DEF_VOLUME,1500,50}, {0,225,0,0},
};
  
#define NO_HANDS_J_LEN 4
static struct aud_req no_hands_j[NO_HANDS_J_LEN] = {
  {1,DEF_VOLUME,1500,50},
  {1,DEF_VOLUME,300,50},
  {1,DEF_VOLUME,1500,50}, {0,225,0,0},
};
  
#define PHONE_J_LEN 4
static struct aud_req phone_j[PHONE_J_LEN] = {
  {1,DEF_VOLUME,300,50},
  {1,DEF_VOLUME,1500,50},
  {1,DEF_VOLUME,300,50}, {0,255,0,0},
};

#define POTHOLE_J_LEN 6
static struct aud_req pothole_j[POTHOLE_J_LEN] = {
  {1,DEF_VOLUME,300,5}, {0,255,0,1},
  {1,DEF_VOLUME,300,5}, {0,255,0,1},
  {1,DEF_VOLUME,300,5}, {0,255,0,1},
};

bool aud_init(void);

bool aud_teardown(void);

bool aud_beep(bool en, int16_t vol , uint16_t freq );

bool send_aud_beep(bool en,int16_t vol, uint16_t freq,uint32_t dur);

bool send_aud_seq(struct aud_req* req,uint8_t len);

#ifdef __cplusplus
}
#endif

#endif  
