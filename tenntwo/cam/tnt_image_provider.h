
#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void camera_init(uint16_t clip_x1, uint16_t clip_y1, uint16_t clip_x2, uint16_t clip_y2);
void camera_teardown();
int getimage(unsigned char *out_data, uint16_t _x1, uint16_t _y1, uint16_t _x2, uint16_t _y2, bool color);

#ifdef __cplusplus
}
#endif

#endif  