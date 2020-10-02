#ifndef SIMPLE_LSB
#define SIMPLE_LSB

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"


// Return values:
// -1 - 0 image size
// -2 - 0 message
// -3 - long message
// 0 - success.
int8_t simple_lsb_encrypt(Image st_img, uint32_t msg_len, const char * restrict msg){
    assert(st_img.img_p != NULL);
    assert(msg != NULL);

    if(st_img.image_size == 0){
        fprintf(stderr, "Error: zero size image provided.\n");
        return -1;
    }
    
    if(msg_len == 0){
        fprintf(stderr, "Error: zero size message provided.\n");
        return -2;
    }

    if(msg_len * NUM_BITS_IN_CHAR > st_img.image_size/st_img.channels){
        fprintf(stderr, "Error: Message too long to store.\n");
        return -3;
    }

    uint8_t bit_value = 0;
    uint8_t blue_index = (st_img.channels == 4) ? 2 : 1;
    for (uint64_t i = 0, j = 0; i != st_img.image_size && j != msg_len * NUM_BITS_IN_CHAR; i += st_img.channels, j += 1){
        bit_value = get_bit_from_char(j % NUM_BITS_IN_CHAR, msg[j / NUM_BITS_IN_CHAR]);       
        st_img.img_p[i + st_img.channels - blue_index] = (st_img.img_p[i + st_img.channels - blue_index] & (~(uint8_t)1)) | bit_value;
    }

    return 0;
}

// Return values:
// -1 - 0 image size
// NULL character not counted in msg_len
int8_t simple_lsb_decrypt(Image st_img, uint32_t msg_len, char * restrict msg){
    assert(st_img.img_p != NULL);
    assert(msg != NULL);

    if(msg_len == 0){
        return 0;
    }
        
    if(st_img.image_size == 0){
        fprintf(stderr, "Error: zero size image provided.\n");
        return -1;
    }

    uint8_t mask = 1;
    uint8_t bit_index = 0;
    uint64_t j = 0;
    char c = 0;
    uint8_t blue_index = (st_img.channels == 4) ? 2 : 1;

    for (uint64_t i = 0; i != st_img.image_size; i += st_img.channels){
            
        c = c | ((st_img.img_p[i + st_img.channels - blue_index] & mask) << bit_index);
        bit_index++;
        if(bit_index == 8){
            msg[j] = c;
            bit_index = 0;
            j++;
            c = 0;

            if(j >= msg_len){
                msg[j] = '\0';
                return 0;
            }
        }
    }

    return 0;
}

#endif
