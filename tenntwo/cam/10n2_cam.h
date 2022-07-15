
#ifndef TNN2_CAM_H
#define TNN2_CAM_H

#ifdef __cplusplus
extern "C" {
#endif

struct cam_req
{
    int8_t num;
    int16_t delay;
    uint16_t clip_x0;
    uint16_t clip_y0;
    uint16_t clip_x1;
    uint16_t clip_y1;
    bool color;
    char dir[5];
};

bool send_cam_req(struct cam_req req);

bool cam_init(void);
bool cam_teardown(void);
bool cam_wait(void);
bool cam_release(void);
extern unsigned char* latest_img_buf;
#ifdef __cplusplus
}
#endif

#endif