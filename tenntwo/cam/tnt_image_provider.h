
#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int getimage(unsigned char *out_data, uint16_t _x1, uint16_t _y1, uint16_t _x2, uint16_t _y2, bool color);

#ifdef __cplusplus
}
#endif

#endif  