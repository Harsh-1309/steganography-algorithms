#ifndef IMAGE
#define IMAGE

#include <stdint.h>
#include <stdlib.h>

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
    grey.img_p = malloc(grey.width * grey.height * grey.channels);
    
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

#endif