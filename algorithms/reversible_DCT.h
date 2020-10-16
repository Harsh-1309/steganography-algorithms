//description: High capacity reversible data hiding scheme based upon discrete cosine transformation
//Author: Yih-Kai Lin


#ifndef REVERSIBLE_DCT
#define REVERSIBLE_DCT

#include <stdint.h>
#include "../image.h"
#include "../util.h"

typedef struct e_rdct {
    Image* st_img;
    rBit_stream* stream;
    uint8_t p;
    int32_t q;
} e_rDCT;

typedef struct d_rdct {
    Image* st_img;
    wBit_stream* stream;
    uint8_t p;
    int32_t q;
} d_rDCT;

// Return values:
// -1 - Small image
// -2 - 0 message
// -3 - Error in greyscale conversion
e_rDCT construct_encrypt_struct(const char * restrict img_path, uint32_t msg_len, 
                                const char * restrict msg, uint8_t p, int32_t q);
void destroy_encrypt_struct(e_rDCT * restrict st);

int8_t reversible_DCT_encrypt(e_rDCT st_data);

// Return values:
// -1 - Small image
// -2 - image not greyscale
// NULL character not counted in msg_len
// msg should be zeroed 
d_rDCT construct_decrypt_struct(const char * restrict img_path, 
                                uint32_t msg_len, uint8_t p, int32_t q);
void destroy_decrypt_struct(d_rDCT * restrict st);
int8_t reversible_DCT_decrypt(d_rDCT st_data);

#endif
