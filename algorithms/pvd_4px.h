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
#include <limits.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"

//2^(kl) <= T <= 2^(kh)
#define K_L 3
#define K_H 4
#define T 15

static uint8_t k_bit_lsb(uint8_t pixel, uint8_t value, uint8_t k){
    assert(k < 8);
    return pixel - (pixel % (uint8_t)pow(2, k)) + value;
}

static uint8_t modified_lsb(int16_t delta, uint8_t lsb_val, uint8_t k){
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

static bool is_error_block(const u8_Quad * restrict vals, float avg_diff){
    uint8_t min_minus_max = max_4(vals->x, vals->y, vals->z, vals->w) 
                            - min_4(vals->x, vals->y, vals->z, vals->w);
    
    return (avg_diff <= T && min_minus_max > 2 * T + 2);
}

static float calc_avg_diff(const u8_Quad * restrict vals){
    uint8_t p_min = min_4(vals->x, vals->y, vals->z, vals->w);
    return ((vals->x - p_min) + (vals->y - p_min) + (vals->w - p_min) + (vals->z - p_min))/3.0;
}

static uint16_t calc_squared_diff(const u8_Quad * restrict q1, const u8_Quad * restrict q2){
    return        
      (q1->x - q2->x)*(q1->x - q2->x) 
    + (q1->y - q2->y)*(q1->y - q2->y) 
    + (q1->z - q2->z)*(q1->z - q2->z) 
    + (q1->w - q2->w)*(q1->w - q2->w);
}

static u8_Quad embed_pixels(u8_Quad old_vals, const char * restrict msg, 
                                  uint32_t * restrict msg_index, uint8_t * restrict bit_num,
                                  bool * restrict skip)
{         
    float avg_diff = calc_avg_diff(&old_vals);

    uint8_t k = T >= avg_diff ? K_L : K_H; 

    //Error block
    if(is_error_block(&old_vals, avg_diff)){
        *skip = true;
        return (u8_Quad){0, 0, 0, 0};
    }
    *skip = false;

    //Simple LSB
    uint8_t val = 0;
    u8_Quad LSB_vals = {};

    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.x = k_bit_lsb(old_vals.x, val, k);
    
    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.y = k_bit_lsb(old_vals.y, val, k);
    
    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.z = k_bit_lsb(old_vals.z, val, k);
    
    val = bits_to_val(msg, k, bit_num, msg_index);
    LSB_vals.w = k_bit_lsb(old_vals.w, val, k);
    
    //Modified LSB
    LSB_vals.x =  modified_lsb(LSB_vals.x - old_vals.x, LSB_vals.x, k);
    LSB_vals.y =  modified_lsb(LSB_vals.y - old_vals.y, LSB_vals.y, k);
    LSB_vals.z =  modified_lsb(LSB_vals.z - old_vals.z, LSB_vals.z, k);
    LSB_vals.w =  modified_lsb(LSB_vals.w - old_vals.w, LSB_vals.w, k);

    //Readjusting procedure
    u8_Quad temp = {};
    int8_t mul[] = {0, 1, -1};
    uint8_t a = pow(2, k);

    u8_Quad min_quad;
    uint16_t min_quad_sd = -1;
    float a_diff = 0;
    uint16_t uitemp = 0;

    for(uint8_t i0 = 0; i0 < 3; i0++){
        temp.x = LSB_vals.x + mul[i0] * a;
        
        for(uint8_t i1 = 0; i1 < 3; i1++){
            temp.y = LSB_vals.y + mul[i1] * a;

            for(uint8_t i2 = 0; i2 < 3; i2++){
                temp.z = LSB_vals.z + mul[i2] * a;

                for(uint8_t i3 = 0; i3 < 3; i3++){
                    temp.w = LSB_vals.w + mul[i3] * a;

                    a_diff = calc_avg_diff(&temp);
                    if((T >= a_diff && avg_diff > T) || (T >= avg_diff && a_diff > T)) continue;
                    if(is_error_block(&temp, a_diff)) continue;
                    uitemp = calc_squared_diff(&temp, &old_vals);

                    printf("x: %u y: %u z: %u w: %u ca: %u \n", temp.x, temp.y, temp.z, temp.w, uitemp);


                    if(uitemp < min_quad_sd){
                        min_quad_sd = uitemp;
                        min_quad = temp;
                    }
                }
            }
        }
    }


    return min_quad;
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

    /*if(msg_len * NUM_BITS_IN_CHAR > st_img->image_size/st_img->channels){
        fprintf(stderr, "Error: Message too long to store.\n");
        return -3;
    }*/

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
    uint8_t *g1 = NULL, *g2 = NULL, *g3 = NULL, *g4 = NULL;

    u8_Quad new_val;

    uint32_t msg_index = 0;
    uint64_t height = (st_img->height - st_img->height % 2);
    uint64_t it_width = (st_img->width - st_img->width % 2) * st_img->channels;
    uint64_t img_buf_width = st_img->width * st_img->channels;

    for(uint64_t j = 0; j < height; j +=2){        
        if(msg_index >= msg_len) break;
        for(uint64_t i = 0; i < it_width; i += 2*st_img->channels){
            if(msg_index >= msg_len) break;

            g1 = &st_img->img_p[i_img(img_buf_width, i, j)];
            g2 = &st_img->img_p[i_img(img_buf_width, i + grey_index + 1, j)];
            g3 = &st_img->img_p[i_img(img_buf_width, i, j + 1)];
            g4 = &st_img->img_p[i_img(img_buf_width, i + grey_index + 1, j + 1)];

            new_val = embed_pixels((u8_Quad){*g1, *g2, *g3, *g4}, msg, 
                                  &msg_index, &bit_num,
                                  &skip);

            if(skip) continue;
            
            *g1 = new_val.x;
            *g2 = new_val.y;
            *g3 = new_val.z;
            *g4 = new_val.w;

        }
    }
    return 0;
}

#endif 