//Description: A steganographic method for images by pixel-value differencing 
//Authors: Da-Chun Wu,Wen-Hsiang Tsai

#ifndef PVD_4PX
#define PVD_4PX

#include <stdint.h>
#include "../image.h"

// Return values:
// -1 - 0 image size
// -2 - 0 message
// -3 - Error in greyscale conversion
int8_t pvd_4px_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg);

// Return values:
// -1 - 0 image size
// -2 - image not greyscale
// NULL character not counted in msg_len
//msg should be zeroed 
int8_t pvd_4px_decrypt(const Image * restrict st_img, uint32_t msg_len, char * restrict msg);

#endif 