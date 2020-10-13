//description: High capacity reversible data hiding scheme based upon discrete cosine transformation
//Author: Yih-Kai Lin


#ifndef REVERSIBLE_DCT
#define REVERSIBLE_DCT

#include <stdint.h>
#include "../image.h"

// Return values:
// -1 - Small image
// -2 - 0 message
// -3 - Error in greyscale conversion
int8_t reversible_DCT_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg);

// Return values:
// -1 - Small image
// -2 - image not greyscale
// NULL character not counted in msg_len
// msg should be zeroed 
int8_t reversible_DCT_decrypt(const Image * restrict st_img, uint32_t msg_len, char * restrict msg);

#endif
