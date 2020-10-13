//Description: High payload steganography mechanism using hybrid edge detector
//Authors: Wen-Jan Chen, Chin-Chen Chang, T. Hoang Ngan Le

#ifndef EDGE_DETECT_LSB
#define EDGE_DETECT_LSB

#include <stdint.h>
#include "../image.h"

// Return values:
// -1 - 0 image size
// -2 - 0 message
// -3 - Error in greyscale conversion
int8_t edge_detect_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg);


// Return values:
// -1 - 0 image size
// -2 - image not greyscale
// NULL character not counted in msg_len
//msg should be zeroed 
int8_t edge_detect_decrypt(const Image * restrict st_img, uint32_t msg_len, char * restrict msg);


#endif