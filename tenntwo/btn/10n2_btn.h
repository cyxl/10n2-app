
#ifndef TENN2_BTN_H
#define TENN2_BTN_H

#ifdef __cplusplus
extern "C" {
#endif


enum tnt_menu{
    top = 0,
    pos,
    img,
    num_menu
};

enum tnt_pos_menu{
    pos_off = 0,
    imu_gnss,
    num_pos_menu, //TODO
    imu,
    gnss,
};

enum tnt_top_menu{
    top_running = 0,
    top_unused1,
    top_unused2,
    top_quit,
    num_top_menu,
};

enum tnt_img_menu{
    cam_off = 0,
    cam_bw_on,
    num_img_menu,  //TODO 
    cam_color_on,
};

extern uint8_t current_menu;
extern uint8_t current_submenu;

bool btn_init(void);
bool btn_teardown(void);

#ifdef __cplusplus
}
#endif
#endif