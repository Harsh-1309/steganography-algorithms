#ifndef IMAGE
#define IMAGE

#include <stdint.h>
#include <stdlib.h>
#include <tgmath.h>
#include <stdbool.h>

#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


enum image_creator{
    MALLOC, 
    LIBRARY
};

typedef struct{
    int32_t width;
    int32_t height;
    int32_t channels;
    uint8_t* img_p;
    uint64_t image_size;
    enum image_creator ic;
} Image;

Image load_image(const char* img_path){
    Image i = {};
    i.img_p = stbi_load(img_path, &i.width, &i.height, &i.channels, 0);
    i.image_size = i.width * i.height * i.channels;
    i.ic = LIBRARY;

    i.ic = LIBRARY;
    return i;
}

void write_png(const char* img_path, Image img){
    stbi_write_png(img_path, img.width, img.height, img.channels, img.img_p, img.width*img.channels);
}

void free_image(Image* img){
    if(img->ic == LIBRARY)
        stbi_image_free(img->img_p);
    else if(img->ic == MALLOC)
        free(img->img_p);

    img->width = img->height = img->channels = img->image_size = 0;
    img->img_p = NULL;
}


Image convert_to_greyscale(const Image* img){
    Image grey = {img->width, img->height, img->channels == 4 ? 2 : 1, NULL, 0, MALLOC}; 
    grey.image_size = grey.width * grey.height * grey.channels;
    grey.img_p = malloc(grey.width * grey.height * grey.channels * sizeof(uint8_t));
    
    if(grey.img_p == NULL){
        fprintf(stderr, "Failed to allocate memory for the greyscale conversion.\n");
        return (Image){0};
    }

    for(uint64_t i = 0, j = 0; i != img->image_size && j != grey.image_size; i += img->channels, j += grey.channels){
        grey.img_p[j] = (img->img_p[i] + img->img_p[i + 1] + img->img_p[i + 2])/3;
        if(grey.channels == 2)
            grey.img_p[j + 1] = img->img_p[i + 3]; 
    }

    return grey;

}

#define W 3
#define TAU 9
#define P 1

Image fuzzy_edge_detector(const Image* img){
    if(img->channels > 2) return (Image){0};

    //Finding the max grey value
    uint8_t max_value = 0;
    for(uint64_t i = 0; i < img->image_size; i += img->channels){
        if(img->img_p[i] > max_value){
            max_value = img->img_p[i];
            if (max_value == 255) break;
        } 
    }

    //Normalisation with WxW neighbour
    float* fnorm_img = malloc(img->image_size/img->channels * sizeof(float));
    
    if(fnorm_img == NULL){
        fprintf(stderr, "Failed to allocate memory for the normalisation.\n");
        return (Image){0};
    } 
/*
    for(uint64_t i = 0, j = 0; i < img->image_size && j < img.image_size/img->channels; i += img->channels, j++){
        fnorm_img[j] = img->img_p[i]/(float)max_value;
    }
*/
    uint64_t width = img->width * img->channels;
    uint8_t min_g = 255;
    uint8_t max_g = 0;
    uint8_t gi = img->channels - 1;
    uint8_t g = 0; 
    
    uint8_t w_half = W/2;
    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < width; i += img->channels){
            
            for(int8_t k = -w_half; k <= w_half; k++){
                for(int8_t l = -w_half; l <= w_half; l++){
                    if((i + l*(gi + 1) != -1) && (j + k != -1) && (i + l*(gi + 1) < width) && (j + k < img->height)){
                        g = img->img_p[i_img(width, i + l*(gi + 1), j + k)];
                        if(g < min_g){
                            min_g = g;
                        }

                        if(g > max_g){
                            max_g = g;
                        }
                    }
                }
            }

            fnorm_img[i_img(img->width, i/img->channels, j)] = (max_g - min_g)/(float)max_value;
            min_g = 255;
            max_g = 0;
        }
    }


    //Edginess
    float sum = 0.0f;
    float f = 0.0f;
    float max = 0.0f;
    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < img->width; i++){
            
            for(int8_t k = -w_half; k <= w_half; k++){
                for(int8_t l = -w_half; l <= w_half; l++){
                    if((i + l != -1) && (j + k != -1) && (i + l < width) && (j + k < img->height)){
                        f = fnorm_img[i_img(img->width, i + l, j + k)];
                        sum += (f * pow(1 - pow(f, P), 1.0/P));
                    }
                }
            }

            fnorm_img[i_img(img->width, i, j)] = fmin(1, (float)TAU/W * sum);
            if(fnorm_img[i_img(img->width, i, j)] > max)
                max = fnorm_img[i_img(img->width, i, j)];
            sum = 0;
        }
    }

    //Hystersis
    Image edge = {img->width, img->height, img->channels, NULL, img->image_size, MALLOC};
    edge.img_p = malloc(edge.image_size);
    
    if(edge.img_p == NULL){
        fprintf(stderr, "Failed to allocate memory for the greyscale conversion.\n");
        return (Image){0};
    }

    for(uint64_t i = 0; i < edge.image_size; i += edge.channels){
        edge.img_p[i] = fnorm_img[i/edge.channels]/max * 255; 
        if(edge.channels == 2)
            edge.img_p[i + 1] = img->img_p[i + 1];
    }

/*
    bool has_strong = false;
    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < width; i += img->channels){
            
            for(int8_t k = -w_half; k <= w_half; k++){
                for(int8_t l = -w_half; l <= w_half; l++){
                    if((i + l*(gi + 1) != -1) && (j + k != -1) && (i + l*(gi + 1) < width) && (j + k < img->height)){
                        has_strong = fnorm_img[i_img(img->width, i + l, j + k)] == 1 ? true : false;
                    }

                    if(has_strong == true) break;
                }
                if(has_strong == true) break;
            }

            if(has_strong){
                edge.img_p[i_img(width, i, j)] = 255;
            }else{
                edge.img_p[i_img(width, i, j)] = 0;
            }

            has_strong = false;
        }
    }
*/

    free(fnorm_img);
    return edge;
}

#endif