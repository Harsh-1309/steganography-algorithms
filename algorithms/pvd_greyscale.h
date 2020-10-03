//Description: A steganographic method for images by pixel-value differencing 
//Authors: Da-Chun Wu,Wen-Hsiang Tsai

#ifndef PVD_GREYSCALE
#define PVD_GREYSCALE

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <tgmath.h>
#include <stdbool.h>
#include <string.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"


//Paritioning scheme 8, 8, 16, 32, 64, 128
static u8_Pair find_difference_range(uint8_t d){
    assert(d <= 255);

    if(d >= 0 && d <= 7) return (u8_Pair){0, 7};
    else if(d >= 8 && d <= 15) return (u8_Pair){8, 15};
    else if(d >= 16 && d <= 31) return (u8_Pair){16, 31};
    else if(d >= 32 && d <= 63) return (u8_Pair){32, 63};
    else if(d >= 64 && d <= 127) return (u8_Pair){64, 127};
    else if(d >= 128 && d <= 255) return (u8_Pair){128, 255};

    assert(0);
}

static u8_Pair embedding_func(u8_Pair i_pixel, int16_t d_old, int16_t d_new, bool *out_bounds){
    assert(out_bounds != NULL);
    int16_t diff = d_new - d_old;
    if(abs(d_old) % 2 == 0){
        int16_t x = (int16_t)(i_pixel.x) - floor(diff/2.0f);
        int16_t y = (int16_t)(i_pixel.y) + ceil(diff/2.0f);
        if( x < 0 || x > 255 ||  y < 0 || y > 255)  *out_bounds = true;
        else *out_bounds = false;

        return (u8_Pair){x, y};
    }

    int16_t x = (int16_t)(i_pixel.x) - ceil(diff/2.0f);
    int16_t y = (int16_t)(i_pixel.y) + floor(diff/2.0f);
    if( x < 0 || x > 255 ||  y < 0 || y > 255)  *out_bounds = true;
    else *out_bounds = false;

    return (u8_Pair){x, y};
}

static u8_Pair calc_new_grey_vals(u8_Pair old_vals, const char * restrict msg, uint32_t msg_len, 
                                  uint32_t * restrict msg_index, uint8_t * restrict bit_num, bool * restrict skip)
{
    uint8_t num_bits;
    int16_t d;
    int16_t d_new;
    u8_Pair range = {};
    bool out_bounds = false;

    d = old_vals.y - old_vals.x;
    range = find_difference_range(abs(d));
    //Checking if the pixel is eligible or not for embedding
    embedding_func(old_vals, abs(d), range.y, &out_bounds);
    if(out_bounds){
        *skip = true;
        return (u8_Pair){0, 0};
    }

    //Embedding data
    num_bits = log2(range.y - range.x + 1);
    if (num_bits == 0){
        *skip = true;
        return (u8_Pair){0, 0};
    }
    *skip = false;

    d_new = range.x + bits_to_val(msg, num_bits, msg_len, bit_num, msg_index);
    //printf("Bits written: %u value written: %u ", num_bits, bits_to_val(&msg[*msg_index], num_bits, *bit_num));
    d_new *= d >= 0 ? 1 : -1;
    

    return embedding_func((u8_Pair){old_vals.x, old_vals.y}, d, d_new, &out_bounds);               
}

static void calc_old_grey_vals(u8_Pair old_vals, uint32_t msg_len, char * restrict msg, 
                                  uint32_t * restrict msg_index, uint8_t * restrict bit_num)
{
    uint8_t num_bits;
    uint8_t bits;
    int16_t d_new;
    u8_Pair range = {};
    bool out_bounds = false;

    d_new = old_vals.y - old_vals.x;

    range = find_difference_range(abs(d_new));
    embedding_func(old_vals, abs(d_new), range.y, &out_bounds);
    if(out_bounds) return;
 
    bits = d_new >= 0 ? d_new - range.x : -d_new - range.x;
    num_bits = log2(range.y - range.x + 1);
    uint8_t bit = 0;
    for(uint8_t i = 0; i < num_bits; i++){
        bit = (bits >> i) & (1);
        if(bit == 1)
            msg[*msg_index] |= (uint8_t)1 << *(bit_num);
        
        (*bit_num)++;
        if((*bit_num) % NUM_BITS_IN_CHAR == 0){
            (*bit_num) = 0;
            (*msg_index)++;
            if((*msg_index) >= msg_len)
                break;
        }
    }
}

// Return values:
// -1 - 0 image size
// -2 - 0 message
// -3 - Error in greyscale conversion
int8_t pvd_grayscale_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg){
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

    bool flip = false;
    bool skip_first_pixel = false;
    bool skip = false;

    uint8_t grey_index = st_img->channels - 1;
    uint8_t bit_num = 0;
    uint8_t *g1 = NULL, *g2 = NULL;
    u8_Pair new_val;

    uint32_t msg_index = 0;
    uint64_t i = 0;
    uint64_t width = st_img->width * st_img->channels;
    for(uint64_t j = 0; j != st_img->height && j < st_img->height; j++){
        if(msg_index >= msg_len) break;

        if(!flip){
            if(skip_first_pixel) i = 1 + grey_index;
            else i = 0;

            for(; i != width && i < width; i += 2*st_img->channels){
                //Checking if full message is embedded or not
                if(msg_index >= msg_len) break;

                //Getting pixel values
                g1 = &(st_img->img_p[i_img(width, i, j)]);
                if(i == width - 1 - grey_index){
                    if(j + 1 == st_img->height) break;

                    g2 = &(st_img->img_p[i_img(width, (width - 1 - grey_index), j + 1)]);
                    skip_first_pixel = true;
                    
                }else{
                    g2 = &(st_img->img_p[i_img(width, i + 1 + grey_index, j)]);
                }

                new_val = calc_new_grey_vals((u8_Pair){*g1, *g2}, msg, msg_len, &msg_index, &bit_num, &skip);
                if(skip == true)
                    continue;

                //printf("hPos: (%lu, %lu) OG: (%u, %u)", i, j, *g1, *g2);
                *g1 = new_val.x;
                *g2 = new_val.y;
                //printf(" NEW: (%u, %u)\n", *g1, *g2);

            }
        }else{
            if(skip_first_pixel) i = (width - 1 - grey_index) -  (1 + grey_index);
            else i = width - 1 - grey_index;

            for(;; i -= 2*st_img->channels){
                //Checking if full message is embedded or not
                if(msg_index >= msg_len) break;

                //Getting pixel values
                g1 = &(st_img->img_p[i_img(width, i, j)]);
                if(i == 0){
                    if(j + 1 == st_img->height) break;

                    g2 = &(st_img->img_p[i_img(width, 0, j + 1)]);
                    skip_first_pixel = true;
                }else{
                    g2 = &(st_img->img_p[i_img(width, i - 1 - grey_index, j)]);
                }

                new_val = calc_new_grey_vals((u8_Pair){*g1, *g2}, msg, msg_len, &msg_index, &bit_num, &skip);
                if(skip == true){
                    if(i < 2*st_img->channels) break;
                    else continue;
                }
                //printf("Pos: (%lu, %lu) OG: (%u, %u)", i, j, *g1, *g2);

                *g1 = new_val.x;
                *g2 = new_val.y;

                //printf(" NEW: (%u, %u)\n", *g1, *g2);
                if (i < 2*st_img->channels) break;
            }
        }

        flip = !flip; 
    }

    if(msg_index < msg_len){
        printf("Full message can't be embedded in the image, embedded first %u characters (bytes) and %u bits.\n", 
               msg_index, bit_num);
        if(bit_num != 0) printf("Recovery key is: %u.\n", msg_index + 1);
        else printf("Recovery key is: %u.\n", msg_index);

    }else printf("Recovery key is: %u.\n", msg_len);

    return 0;
}

// Return values:
// -1 - 0 image size
// NULL character not counted in msg_len
int8_t pvd_grayscale_decrypt(const Image * restrict st_img, uint32_t msg_len, char * restrict msg){
    assert(st_img->img_p != NULL);
    assert(msg != NULL);

    if(msg_len == 0){
        return 0;
    }

    if(st_img->image_size == 0){
        fprintf(stderr, "Error: zero size image provided.\n");
        return -1;
    }
    bool flip = false;
    bool skip_first_pixel = false;

    uint8_t grey_index = st_img->channels - 1;
    uint8_t bit_num = 0;
    uint8_t g1, g2;

    uint32_t msg_index = 0;
    uint64_t i = 0;
    uint64_t width = st_img->width * st_img->channels;
    for(uint64_t j = 0; j != st_img->height && j < st_img->height; j++){
        if(msg_index >= msg_len) break;

        if(!flip){
            if(skip_first_pixel) i = 1 + grey_index;
            else i = 0;

            for(; i != width && i < width; i += 2*st_img->channels){
                //Checking if full message is embedded or not
                if(msg_index >= msg_len) break;

                //Getting pixel values
                g1 = (st_img->img_p[i_img(width, i, j)]);
                if(i == width - 1 - grey_index){
                    if(j + 1 == st_img->height) break;

                    g2 = (st_img->img_p[i_img(width, (width - 1 - grey_index), j + 1)]);
                    skip_first_pixel = true;
                }else{
                    g2 = (st_img->img_p[i_img(width, i + 1 + grey_index, j)]);
                }

                calc_old_grey_vals((u8_Pair){g1, g2}, msg_len, msg, &msg_index, &bit_num);
            }
        }else{
            if(skip_first_pixel) i = (width - 1 - grey_index) -  (1 + grey_index);
            else i = width - 1 - grey_index;

            for(;; i -= 2*st_img->channels){
                //Checking if full message is embedded or not
                if(msg_index >= msg_len) break;

                //Getting pixel values
                g1 = (st_img->img_p[i_img(width, i, j)]);
                if(i == 0){
                    if(j + 1 == st_img->height) break;

                    g2 = (st_img->img_p[i_img(width, 0, j + 1)]);
                    skip_first_pixel = true;
                }else{
                    g2 = (st_img->img_p[i_img(width, i - 1 - grey_index, j)]);
                }

                calc_old_grey_vals((u8_Pair){g1, g2}, msg_len, msg, &msg_index, &bit_num);

                if (i < 2*st_img->channels) break;
            }
        }

        flip = !flip; 
    }

    return 0;
}

#endif 