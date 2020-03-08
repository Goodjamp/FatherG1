#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "stdint.h"

typedef struct Image{
    const uint8_t *image;
    uint32_t       width;
    uint32_t       height;
    uint32_t       size;
} Image;

#endif
