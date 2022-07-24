
#ifndef TENN2_BTN_H
#define TENN2_BTN_H

#ifdef __cplusplus
extern "C" {
#endif


enum tnt_men{
    top = 0,
    inf,
    train,
    num_menu
};

enum tnt_top_menu{
    top_unused1,
    top_unused2,
    top_unused3,
    top_quit,
    num_top_menu,
};

enum tnt_train_menu{
    cam_off = 0,
    cam_hands_on,
    cam_cell_on,
    cam_none_on,
    num_train_menu,  
};

enum tnt_inf_menu{
    inf_off=0,
    inf_on,
    inf_rec_terse,
    inf_rec_verbose,
    num_inf_menu,  
};
extern uint8_t current_menu;
extern uint8_t current_submenu;

bool btn_init(void);
bool btn_teardown(void);

#ifdef __cplusplus
}
#endif
#endif