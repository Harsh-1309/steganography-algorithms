//Description: A steganographic method for images by pixel-value differencing 
//Authors: Da-Chun Wu,Wen-Hsiang Tsai

#ifndef PVD_4PX
#define PVD_4PX

#include <stdint.h>
#include "../image.h"
#include "../util.h"


typedef struct e_pvd4x {
    Image* st_img;
    rBit_stream* stream;
    uint8_t k_l;
    uint8_t k_h;
    uint8_t t;
} e_PVD4x;

typedef struct d_pvd4x {
    Image* st_img;
    wBit_stream* stream;
    uint8_t k_l;
    uint8_t k_h;
    uint8_t t;
} d_PVD4x;


e_PVD4x construct_e_PVD4x_struct(const char * restrict img_path, uint32_t msg_len,
                                 const char * restrict msg, uint8_t k_l, uint8_t k_h, uint8_t t);
void destroy_e_PVD4x_struct(e_PVD4x * restrict st);
void pvd_4px_encrypt(e_PVD4x st_data);


d_PVD4x construct_d_PVD4x_struct(const char * restrict img_path, uint32_t msg_len, 
                                 uint8_t k_l, uint8_t k_h, uint8_t t);
void destroy_d_PVD4x_struct(d_PVD4x * restrict st);
void pvd_4px_decrypt(d_PVD4x st_data);

#endif 