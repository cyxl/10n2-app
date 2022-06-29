
#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int getimage(unsigned char *out_data,uint16_t vsize,uint16_t hsize,bool color);

#ifdef __cplusplus
}
#endif

#endif  