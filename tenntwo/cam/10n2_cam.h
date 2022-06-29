
#ifndef TNN2_CAM_H
#define TNN2_CAM_H

#ifdef __cplusplus
extern "C" {
#endif

struct cam_req
{
    int8_t num;
    int16_t delay;
    uint16_t width;
    uint16_t height;
    bool color;
    char dir[5];
};

bool send_cam_req(struct cam_req req);

bool cam_init(void);
bool cam_teardown(void);

#ifdef __cplusplus
}
#endif

#endif