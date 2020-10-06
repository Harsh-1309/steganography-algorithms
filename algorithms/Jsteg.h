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

static void write_start_of_file_marker(FILE* fout){
    const uint8_t SOI_MARKER_B1 = 0xFF;
    const uint8_t SOI_MARKER_B2 = 0xD8;

    fwrite(&SOI_MARKER_B1, sizeof(SOI_MARKER_B1), 1, fout);
    fwrite(&SOI_MARKER_B2, sizeof(SOI_MARKER_B2), 1, fout);
}

static void write_APP0_marker(FILE* fout){
    const uint8_t APP0_MARKER_B1 = 0xFF;
    const uint8_t APP0_MARKER_B2 = 0xE0;
    const uint8_t APP0_SIZE_B1 = 0x00;
    const uint8_t APP0_SIZE_B2 = 0x10;
    const uint8_t JFIF_B1 = 0x4a;
    const uint8_t JFIF_B2 = 0x46;
    const uint8_t JFIF_B3 = 0x49;
    const uint8_t JFIF_B4 = 0x46;
    const uint8_t JFIF_B5 = 0x00;
    const uint8_t MAJOR_VERSION = 0x01;
    const uint8_t MINOR_VERSION = 0x02;
    const uint8_t PIXEL_DENSITY_UNIT = 0x01;
    const uint8_t HORIZONTAL_PIXEL_DENSITY_B1 = 0x10;
    const uint8_t HORIZONTAL_PIXEL_DENSITY_B2 = 0x00;
    const uint8_t VERTICAL_PIXEL_DENSITY_B1 = 0x10;
    const uint8_t VERTICAL_PIXEL_DENSITY_B2 = 0x00;
    const uint8_t THUMBNAIL_WIDTH = 0x00;
    const uint8_t THUMBNAIL_HEIGHT = 0x00;

    const uint8_t app0_data[APP0_SIZE + 2] = {
        APP0_MARKER_B1, APP0_MARKER_B2,
        APP0_SIZE_B1, APP0_SIZE_B2,
        JFIF_B1, JFIF_B2, JFIF_B3, JFIF_B4, JFIF_B5,
        MAJOR_VERSION, MINOR_VERSION,
        PIXEL_DENSITY_UNIT,
        HORIZONTAL_PIXEL_DENSITY_B1, HORIZONTAL_PIXEL_DENSITY_B2,
        VERTICAL_PIXEL_DENSITY_B1, VERTICAL_PIXEL_DENSITY_B2,
        THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT
    };

    fwrite(app0_data, size(uint8_t), APP0_SIZE + 2, fout);
}

static void write_comment_marker(FILE* fout){
    const uint8_t COM_MARKER_B1 = 0xFF;
    const uint8_t COM_MARKER_B2 = 0xE0;
    
    const uint8_t app0_data

}

// Return values:
// -1 - 0 image size
// -2 - 0 message
int8_t jsteg_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg, const char * restrict path){


    



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

    FILE* fout = NULL;
    fout = fopen(path, "wb");
    
    //SOI marker
    write_start_of_file_marker(fout);
    
    //APP0 MARKER
    write_APP0_marker(fout);

    //COM MARKER


    



    fclose(fout);
    return 0;
}