//Description: High payload steganography mechanism using hybrid edge detector
//Authors: Wen-Jan Chen, Chin-Chen Chang, T. Hoang Ngan Le

#ifndef EDGE_DETECT_LSB
#define EDGE_DETECT_LSB

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <tgmath.h>
#include <stdbool.h>
#include <string.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"

const uint8_t block_size = 3;
const uint8_t non_edge_bits = 1;
const uint8_t edge_bits = 5; 

// Return values:
// -1 - 0 image size
// -2 - 0 message
// -3 - Error in greyscale conversion
int8_t edge_detect_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg){
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
            return -3;
        }
    }

    Image edge = hybrid_edge_detector(st_img);

    const uint8_t channels = st_img->channels;
    uint8_t bit_num = 0;

    uint32_t msg_index = 0;
    const uint64_t size = edge.image_size - (edge.image_size % block_size); 

    uint8_t block_id = 0;
    uint8_t bits = 0;
    for(uint64_t i = 0; i < size; i += block_size * channels){
        if(msg_index >= msg_len) break;

        for(uint8_t j = 1; j < block_size; j++){
            if(msg_index >= msg_len) break;

            if(edge.img_p[i + j * channels] == 255){
                block_id += power_2(j - 1);
                bits = bits_to_val(msg, edge_bits, msg_len, &bit_num, &msg_index);
                st_img->img_p[i + j * channels] = k_bit_lsb(st_img->img_p[i + j * channels], bits, edge_bits);
            }else{
                bits = bits_to_val(msg, non_edge_bits, msg_len, &bit_num, &msg_index);
                st_img->img_p[i + j * channels] = k_bit_lsb(st_img->img_p[i + j * channels], bits, non_edge_bits);
            }
        }

        st_img->img_p[i] = k_bit_lsb(st_img->img_p[i], block_id, block_size - 1);
        block_id = 0;
    }

    if(msg_index < msg_len){
        printf("Full message can't be embedded in the image, embedded first %u characters (bytes) and %u bits.\n", 
               msg_index, bit_num);
        if(bit_num != 0) printf("Recovery key is: %u.\n", msg_index + 1);
        else printf("Recovery key is: %u.\n", msg_index);

    }else printf("Recovery key is: %u.\n", msg_len);

    free_image(&edge);

    return 0;
}


// Return values:
// -1 - 0 image size
// -2 - image not greyscale
// NULL character not counted in msg_len
//msg should be zeroed 
int8_t edge_detect_decrypt(const Image * restrict st_img, uint32_t msg_len, char * restrict msg){
    assert(st_img->img_p != NULL);
    assert(msg != NULL);

    if(msg_len == 0){
        return 0;
    }

    if(st_img->image_size == 0){
        fprintf(stderr, "Error: zero size image provided.\n");
        return -1;
    }

    if(st_img->channels > 2) return -2;

    const uint8_t channels = st_img->channels;
    uint8_t bit_num = 0;

    uint32_t msg_index = 0;
    const uint64_t size = st_img->image_size - st_img->image_size % block_size; 

    uint8_t block_id = 0;
    uint8_t bits = 0;

    for(uint64_t i = 0; i < size; i += block_size * channels){
        if(msg_index >= msg_len) break;

        block_id = recover_k_bit_lsb(st_img->img_p[i], block_size - 1);

        for(uint8_t j = 1; j < block_size; j++, block_id >>= 1){
            if(msg_index >= msg_len) break;
            if((block_id >> 1) == 1){
                bits = recover_k_bit_lsb(st_img->img_p[i + j * channels], edge_bits);
                embed_bits_to_msg(msg, &msg_index, &bit_num, bits, edge_bits, msg_len);
            }else{
                bits = recover_k_bit_lsb(st_img->img_p[i + j * channels], non_edge_bits);
                embed_bits_to_msg(msg, &msg_index, &bit_num, bits, non_edge_bits, msg_len);
            }

        }
    }

    return 0;
}


#endif