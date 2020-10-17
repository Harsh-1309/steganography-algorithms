#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <tgmath.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"
#include "pvd_4px.h"




static uint8_t modified_lsb(int16_t delta, uint8_t lsb_val, uint8_t k){
    assert(k < 8);
    
    const uint8_t a = power_2(k - 1);
    const uint8_t b = power_2(k);

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

static bool is_error_block(const u8_Quad * restrict vals, float avg_diff, uint8_t t){

    uint8_t min_minus_max = max_4(vals->x, vals->y, vals->z, vals->w) 
                            - min_4(vals->x, vals->y, vals->z, vals->w);
    
    if(avg_diff <= t && min_minus_max > 2 * t + 2){
        return true;
    }

    return false;
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

static u8_Quad embed_data(u8_Quad old_vals, e_PVD4x st_data, bool * restrict skip)
{         
    float avg_diff = calc_avg_diff(&old_vals);

    uint8_t k = st_data.t >= avg_diff ? st_data.k_l : st_data.k_h; 

    //Error block
    if(is_error_block(&old_vals, avg_diff, st_data.t)){
        *skip = true;
        return (u8_Quad){0, 0, 0, 0};
    }
    *skip = false;

    //Simple LSB
    uint8_t val = 0;
    u8_Quad LSB_vals = {};

    val = get_bits(st_data.stream, k);
    LSB_vals.x = k_bit_lsb(old_vals.x, val, k);
    
    val = get_bits(st_data.stream, k);
    LSB_vals.y = k_bit_lsb(old_vals.y, val, k);
    
    val = get_bits(st_data.stream, k);
    LSB_vals.z = k_bit_lsb(old_vals.z, val, k);
    
    val = get_bits(st_data.stream, k);
    LSB_vals.w = k_bit_lsb(old_vals.w, val, k);
    
    //Modified LSB
    LSB_vals.x =  modified_lsb(LSB_vals.x - old_vals.x, LSB_vals.x, k);
    LSB_vals.y =  modified_lsb(LSB_vals.y - old_vals.y, LSB_vals.y, k);
    LSB_vals.z =  modified_lsb(LSB_vals.z - old_vals.z, LSB_vals.z, k);
    LSB_vals.w =  modified_lsb(LSB_vals.w - old_vals.w, LSB_vals.w, k);

    //Readjusting procedure
    u8_Quad temp = {};
    int8_t mul[] = {0, 1, -1};
    uint8_t a = power_2(k);

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
                    if((st_data.t >= a_diff && avg_diff > st_data.t) 
                    || (st_data.t >= avg_diff && a_diff > st_data.t)) continue;
                    
                    if(is_error_block(&temp, a_diff, st_data.t)) continue;
                    uitemp = calc_squared_diff(&temp, &old_vals);
                    
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


static void recover_data(u8_Quad old_vals, d_PVD4x st_data)

{
    float avg_diff = calc_avg_diff(&old_vals);
    uint8_t k = st_data.t >= avg_diff ? st_data.k_l : st_data.k_h;
    
    if(is_error_block(&old_vals, avg_diff, st_data.t)) return;
    
    uint8_t pixels[4] = {old_vals.x, old_vals.y, old_vals.z, old_vals.w};
    uint8_t bits = 0;
    for(uint8_t i = 0; i < 4; i++){
        bits = recover_k_bit_lsb(pixels[i], k);
        write_bits(st_data.stream, bits, k);
    }
} 


void pvd_4px_encrypt(e_PVD4x st_data){
    assert(st_data.st_img != NULL);
    assert(st_data.stream != NULL);

    bool skip = false;

    Image* st_img = st_data.st_img;
    const uint8_t channels = st_img->channels;
    uint8_t *g1 = NULL, *g2 = NULL, *g3 = NULL, *g4 = NULL;

    u8_Quad new_val;

    const uint64_t height = (st_img->height - st_img->height % 2);
    const uint64_t it_width = (st_img->width - st_img->width % 2) * st_img->channels;
    const uint64_t img_buf_width = st_img->width * st_img->channels;

    for(uint64_t j = 0; j < height; j +=2){        
        if(!get_rBit_stream_status(st_data.stream)) break;
        for(uint64_t i = 0; i < it_width; i += 2*st_img->channels){

            g1 = &st_img->img_p[i_img(img_buf_width, i, j)];
            g2 = &st_img->img_p[i_img(img_buf_width, i + channels, j)];
            g3 = &st_img->img_p[i_img(img_buf_width, i, j + 1)];
            g4 = &st_img->img_p[i_img(img_buf_width, i + channels, j + 1)];

            new_val = embed_data((u8_Quad){*g1, *g2, *g3, *g4}, st_data, &skip);

            if(skip) continue;
            
            *g1 = new_val.x;
            *g2 = new_val.y;
            *g3 = new_val.z;
            *g4 = new_val.w;

            if(!get_rBit_stream_status(st_data.stream)) break;
        }
    }

    recovery_key_msg(st_data.stream);
}

void pvd_4px_decrypt(d_PVD4x st_data){
    assert(st_data.st_img != NULL);
    assert(st_data.stream != NULL);

    Image* st_img = st_data.st_img;
    const uint8_t channels = st_img->channels;
    uint8_t g1, g2, g3, g4;

    const uint64_t height = (st_img->height - st_img->height % 2);
    const uint64_t it_width = (st_img->width - st_img->width % 2) * st_img->channels;
    const uint64_t img_buf_width = st_img->width * st_img->channels;

    for(uint64_t j = 0; j < height; j +=2){        
        if(!get_wBit_stream_status(st_data.stream)) break;
        for(uint64_t i = 0; i < it_width; i += 2*st_img->channels){


            g1 = st_img->img_p[i_img(img_buf_width, i, j)];
            g2 = st_img->img_p[i_img(img_buf_width, i + channels, j)];
            g3 = st_img->img_p[i_img(img_buf_width, i, j + 1)];
            g4 = st_img->img_p[i_img(img_buf_width, i + channels, j + 1)];

            recover_data((u8_Quad){g1, g2, g3, g4}, st_data);
            
            if(!get_wBit_stream_status(st_data.stream)) break;
        }
    }

}

void destroy_e_PVD4x_struct(e_PVD4x * restrict st){
    assert(st != NULL);

    free_image(st->st_img);
    free(st->st_img);
    delete_read_bitstream(st->stream);
}


e_PVD4x construct_e_PVD4x_struct(const char * restrict img_path, uint32_t msg_len,
                                 const char * restrict msg, uint8_t k_l, uint8_t k_h, uint8_t t){
    
    assert(img_path != NULL);
    assert(msg != NULL);
    assert(msg_len != 0);

    if(k_l > 5 || k_l == 0){
        fprintf(stderr, "Invalid value for k_l.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};        
    }

    if(k_h > 5 || k_h == 0){
        fprintf(stderr, "Invalid value for k_h.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};        
    }

    if(k_h < k_l){
        fprintf(stderr, "k_h is smaller than k_l.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};      
    }

    if(power_2(k_l) > t || t > power_2(k_h)){
        fprintf(stderr, "Invalid value for t.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};            
    }

    e_PVD4x st;
    Image* img = malloc(sizeof(Image));
    if(img == NULL){
        fprintf(stderr, "Unable to allocate memory for image.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};
    }

    *(img) = load_image(img_path);
    if(img->img_p == NULL){
        fprintf(stderr, "Unable to allocate memory for image.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};
    }

    //Convert to greyscale
    if(img->channels > 2){
        Image grey = convert_to_greyscale(img);

        if(grey.img_p == NULL){
            fprintf(stderr, "Error in greycale conversion.\n");
            return (e_PVD4x){NULL, NULL, 0, 0};
        }

        free_image(img);
        *(img) = grey;
    }    

    rBit_stream* stream = create_read_bitstream(msg, msg_len);
    if(stream == NULL){
        fprintf(stderr, "Unable to allocate memory for message bit stream.\n");
        return (e_PVD4x){NULL, NULL, 0, 0, 0};
    }

    st.st_img = img;
    st.stream = stream;
    st.k_l = k_l;
    st.k_h = k_h;
    st.t = t;

    return st;
}

void destroy_d_PVD4x_struct(d_PVD4x * restrict st){
    assert(st != NULL);

    free_image(st->st_img);
    free(st->st_img);
    delete_write_bitstream(st->stream);    
}

d_PVD4x construct_d_PVD4x_struct(const char * restrict img_path, uint32_t msg_len, 
                                 uint8_t k_l, uint8_t k_h, uint8_t t){
    assert(img_path != NULL);
    assert(msg_len != 0);

    if(k_l > 5 || k_l == 0){
        fprintf(stderr, "Invalid value for k_l.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};        
    }

    if(k_h > 5 || k_h == 0){
        fprintf(stderr, "Invalid value for k_h.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};        
    }

    if(k_h < k_l){
        fprintf(stderr, "k_h is smaller than k_l.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};      
    }

    if(power_2(k_l) > t || t > power_2(k_h)){
        fprintf(stderr, "Invalid value for t.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};            
    }

    d_PVD4x st;
    Image* img = malloc(sizeof(Image));
    if(img == NULL){
        fprintf(stderr, "Unable to allocate memory for image.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};
    }

    *(img) = load_image(img_path);
    if(img->img_p == NULL){
        fprintf(stderr, "Unable to allocate memory for image.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};
    }
    
    //check greyscale
    if(img->channels > 2){
        fprintf(stderr, "Not a greyscale image.\n");
        return (d_PVD4x){NULL, NULL, 0, 0};
    }

    wBit_stream* stream = create_write_bitstream(msg_len);
    if(stream == NULL){
        fprintf(stderr, "Unable to allocate memory for message bit stream.\n");
        return (d_PVD4x){NULL, NULL, 0, 0, 0};
    }

    st.st_img = img;
    st.stream = stream;
    st.k_l = k_l;
    st.k_h = k_h;
    st.t = t;

    return st;
}