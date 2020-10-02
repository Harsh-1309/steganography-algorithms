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

    bool flip = false;


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