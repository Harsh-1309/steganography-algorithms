//Description: A steganographic method for images by pixel-value differencing 
//Authors: Da-Chun Wu,Wen-Hsiang Tsai

#ifndef PVD_4PX
#define PVD_4PX

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <tgmath.h>
#include <stdbool.h>
#include <string.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"

//2^(kl) <= T <= 2^(kh)
#define K_L 3
#define K_H 4
#define T 15

static uint8_t k_bit_lsb(uint8_t pixel, uint8_t value, uint8_t k){
    assert(k < 8);
    return pixel - (pixel % pow(2, k)) + value;
}

static uint8_t modified_lsb(uint8_t delta, uint8_t lsb_val, uint8_t k){
    assert(k < 8);
    
    uint8_t a = pow(2, k - 1);
    uint8_t b = pow(2, k);

    assert(-b < delta && delta < b);

    if(a < delta && delta < b){
        if(lsb_val >= b){
            return lsb_val - b;
        }else{
            return lsb_val;
        }
    }else if(-a <= delta && delta <= a){
        return lsb_val;
    }else if(-b <= delta && delta < -a){
        if(lsb_val < 256 - b){
            return lsb_val + b;
        }else{
            return lsb_val;
        }
    }

    assert(0);
}

static bool is_error block(const u8_Quad * restrict vals, uint8_t avg_diff){
    uint8_t min_minus_max = max_4(vals->x, vals->y, vals->z, vals->w) 
                            - min_4(vals->x, vals->y, vals->z, vals->w);
    
    return (avg_diff <= T && min_minus_max > 2 * T + 2);
}

static void calc_new_grey_vals(u8_Quad* old_vals, const char * restrict msg, 
                                  uint32_t * restrict msg_index, uint8_t * restrict bit_num)
{         
    uint8_t p_min = min_4(old_vals->x, old_vals->y, old_vals->z, old_vals->w);
    uint8_t avg_diff = ((old_vals->x - p_min) + (old_vals->y - p_min) + (old_vals->w - p_min) + (old_vals->z - p_min))/3;

    uint8_t k = T >= avg_diff ? K_L : K_H; 

    //Error block
    if(is_error(old_vals, avg_diff)) return;

    //Simple LSB 
    uint8_t bit_num = 0;
    uint8_t val = 0;
    u8_Quad LSB_vals = {};

    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.x = simple_lsb(old_vals->x, val, k);
    
    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.y = simple_lsb(old_vals->y, val, k);
    
    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.z = simple_lsb(old_vals->z, val, k);
    
    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.w = simple_lsb(old_vals->w, val, k);
    
    //Modified LSB
    LSB_vals.x =  modified_lsb(LSB_vals.x - old_vals[i], LSB_vals.x, k);
    LSB_vals.y =  modified_lsb(LSB_vals.y - old_vals[i], LSB_vals.x, k);
    LSB_vals.z =  modified_lsb(LSB_vals.z - old_vals[i], LSB_vals.x, k);
    LSB_vals.w =  modified_lsb(LSB_vals.w - old_vals[i], LSB_vals.x, k);

    //Readjusting procedure


}

// Return values:
// -1 - 0 image size
// -2 - 0 message
// -3 - long message
// -4 - Error in greyscale conversion
int8_t pvd_4px_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg){
    assert(st_img->img_p != NULL);
    assert(msg != NULL);

    if(st_img->image_size == 0){
        fprintf(stderr, "Error: zero size image provided.\n");
        return -1;
    }
    
    if(msg_len == 0){
        fprintf(stderr, "Error: zero size message provided.\n");
        return -2;
    }

    if(msg_len * NUM_BITS_IN_CHAR > st_img->image_size/st_img->channels){
        fprintf(stderr, "Error: Message too long to store.\n");
        return -3;
    }

    if(st_img->channels > 2){
        Image grey = convert_to_greyscale(st_img);
        free_image(st_img);
        st_img->width = grey.width;
        st_img->height = grey.height;
        st_img->channels = grey.channels;
        st_img->img_p = grey.img_p;
        st_img->image_size = grey.image_size;
        st_img->ic = grey.ic;

        if(st_img->img_p == NULL){
            fprintf(stderr, "Error in greycale conversion.\n");
            return -4;
        }
    }

    bool skip = false;


    uint8_t grey_index = st_img->channels - 1;
    uint8_t bit_num = 0;
    uint8_t *g1 = NULL, *g2 = NULL;
    u8_Pair new_val;

    uint32_t msg_index = 0;
    uint64_t i = 0;
    uint64_t height = (st_img->height - st_img->height % 2);
    uint64_t it_width = (st_img->width - st_img->width % 2) * st_img->channels;
    uint64_t img_buf_width = st_img->width * st_img->channels;

    uint8_t k = 0;
    for(uint64_t j = 0; j < height; j +=2){        
        for(uint64_t i = 0; i < it_width; i += 2*st_img->channels){
            st_img->img_p[i_img(img_buf_width, i, j)] = k;
            st_img->img_p[i_img(img_buf_width, i + grey_index + 1, j)] = k;
            st_img->img_p[i_img(img_buf_width, i, j + 1)] = k;
            st_img->img_p[i_img(img_buf_width, i + grey_index + 1, j + 1)] = k;

            k += 5;
        }
    }
    return 0;
}

#endif 