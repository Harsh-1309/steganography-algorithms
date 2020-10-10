//description: High capacity reversible data hiding scheme based upon discrete cosine transformation
//Author: Yih-Kai Lin


#ifndef REVERSIBLE_DCT
#define REVERSIBLE_DCT

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <tgmath.h>
#include <stdbool.h>
#include <string.h>

#include "../constants.h"
#include "../image.h"
#include "../util.h"

#define mat_width 8
#define mat_height 8
#define P 1
#define Q 4


const long double s0[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000,    
    -1.448, -1.620, +2.041, -0.184, +1.351, +1.518, +0.914, +1.000 
};

const long double i_s0[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000,    
    +1.448, +1.620, -2.041, +0.184, -1.351, -1.518, -0.914, +1.000 
};

const long double s1[mat_height * mat_width] = {
    +1.000, +1.204, -1.697, +0.216, -1.291, -0.910, -1.202, +0.750,
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s1[mat_height * mat_width] = {
    +1.000, -1.204, +1.697, -0.216, +1.291, +0.910, +1.202, -0.750,
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s2[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000,
    -0.832, +1.000, -0.949, +0.476, -1.004, -0.753, -1.001, +0.624, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s2[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000,
    +0.832, +1.000, +0.949, -0.476, +1.004, +0.753, +1.001, -0.624, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s3[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.575, +0.692, +1.000, -0.757, +0.248, +0.377, +0.675, -0.432, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s3[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    -0.575, -0.692, +1.000, +0.757, -0.248, -0.377, -0.675, +0.432, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s4[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    -0.459, -0.986, +0.356, +1.000, -0.778, -0.792, -0.644, +0.340, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s4[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.459, +0.986, -0.356, +1.000, +0.778, +0.792, +0.644, -0.340, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s5[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.380, +0.632, +0.070, +0.242, +1.000, -0.024, +0.163, -0.319,
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s5[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    -0.380, -0.632, -0.070, -0.242, +1.000, +0.024, -0.163, +0.319,
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s6[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.129, -0.687, +0.522, +0.688, +1.438, +1.000, +0.720, +0.310, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s6[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    -0.129, +0.687, -0.522, -0.688, -1.438, +1.000, -0.720, -0.310, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s7[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.489, -1.395, +0.764, +0.466, +1.682, -1.154, +1.000, +0.634,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double i_s7[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    -0.489, +1.395, -0.764, -0.466, -1.682, +1.154, +1.000, -0.634,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000   
};

const long double s8[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000,
    -0.044, -2.200, +1.177, +0.575, +2.099, -1.263, -1.152, -1.000   
};

const long double i_s8[mat_height * mat_width] = {
    +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000, +0.000, 
    +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000, +0.000,
    +0.000, +0.000, +0.000, +0.000, +0.000, +0.000, +1.000, +0.000,
    -0.044, -2.200, +1.177, +0.575, +2.099, -1.263, -1.152, -1.000   
};

const long double p[mat_height * mat_width] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 
    0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
    1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 
};

const long double i_p[mat_height * mat_width] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 
    0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 
};


static void round_mat(const long double f[mat_height * mat_width], long double o[mat_height * mat_width]){
    assert(f != NULL);
    assert(o != NULL);

    for(uint8_t i = 0; i < mat_height * mat_width; i++){
        o[i] = round(f[i]);
    }
}

static void mat_mult(const long double a[mat_height * mat_width], const long double b[mat_height * mat_width], 
                     long double o[mat_width * mat_height]){

    assert(a != NULL);
    assert(b != NULL);
    assert(o != NULL);
    assert(mat_height == mat_width);
    
    long double sum = 0.0f;
    for(uint8_t j = 0; j < mat_height; j++){
        for(uint8_t i = 0; i < mat_width; i++){

            for(uint8_t k = 0; k < mat_width; k++){
                sum += a[i_img(mat_width, k, i)] * b[i_img(mat_width, j, k)];
            }

            o[i_img(mat_width, j, i)] = sum;
            sum = 0.0;

        }
    }

}

static void transpose(const long double a[mat_height * mat_width], long double o[mat_height * mat_width]){
    for(uint8_t j = 0; j < mat_height; j++){
        for(uint8_t i = 0; i < mat_width; i++){
            o[i_img(mat_width, j, i)] = a[i_img(mat_width, i, j)];
        }
    }
}

static void copy_mat(const long double src[mat_height * mat_width], long double dest[mat_height * mat_width]){
    for(uint8_t i = 0; i < mat_height * mat_width; i++){
        dest[i] = src[i];
    }
}
/*
static void print_mat(const long double a[mat_width * mat_height]){
    for(uint8_t j = 0; j < mat_height; j++){
        for(uint8_t i = 0; i < mat_width; i++){
            printf("%.1f ", a[i_img(mat_width, i, j)]);
        }
        printf("\n");
    }
}*/

static void transform_subimage(const long double a[mat_height * mat_width], long double o[mat_height * mat_width]){    
    long double temp[mat_height * mat_width];
    long double temp2[mat_height * mat_width];
    const long double * const transforms[8] = {s1, s2, s3, s4, s5, s6, s7, s8};
    
    copy_mat(a, temp2);
    //print_mat(temp2);
    for(uint8_t i = 0; i < 2; i++){
        //print_mat(s0);
        mat_mult(s0, temp2, temp);
        //print_mat(temp);
        round_mat(temp, o);
        //printf("0\n");
        //print_mat(o); printf("\n");

        for(uint8_t j = 0; j < 8; j++){
            mat_mult(transforms[j], o, temp);
            round_mat(temp, o);
            //printf("%u\n", j+1); print_mat(o);
        }

        mat_mult(p, o, temp);
        //printf("P\n"); print_mat(temp);
        transpose(temp, temp2);
        //printf("T\n"); print_mat(temp2);
    }

    transpose(temp2, o);
}

static void invsere_transform_subimage(const long double a[mat_height * mat_width], long double o[mat_height * mat_width]){
    long double temp[mat_height * mat_width];
    long double temp2[mat_height * mat_width];
    const long double * const inverse_transforms[9] = {i_s0, i_s1, i_s2, i_s3, i_s4, i_s5, i_s6, i_s7, i_s8};
    
    copy_mat(a, temp2);
    for(uint8_t i = 0; i < 2; i++){
        mat_mult(i_p, temp2, o);
        for(uint8_t j = 8; j != (uint8_t)-1; j--){
            mat_mult(inverse_transforms[j], o, temp);
            round_mat(temp, o);
        }

        transpose(o, temp2);
    }

    transpose(temp2, o);
}

static uint8_t get_next_bit(const char* restrict msg, uint32_t msg_len, uint32_t * restrict msg_index,
                            uint8_t* bit_num)
{
    assert(*msg_index < msg_len);
    uint8_t bit = (msg[*msg_index] >> *bit_num) & (1);
    (*bit_num)++;
    if((*bit_num) == NUM_BITS_IN_CHAR){
        (*bit_num) = 0;
        (*msg_index)++;
    }

    return bit;
}

static void embed_reversible_pixels(long double dct[mat_height * mat_width], const char * restrict msg,
                                    uint32_t msg_len, uint32_t * restrict msg_index, 
                                    uint8_t * restrict bit_num)
{ 
    assert(dct != NULL);
    assert(msg != NULL);
    assert(msg_index != NULL);
    assert(bit_num != NULL);

    uint8_t bit;
    for(uint8_t j = 0; j < mat_height; j++){
        if(*msg_index >= msg_len) break;

        for(uint8_t i = 0; i < mat_width; i++){
            if (*msg_index >= msg_len) break;
            
            if((i + j) <= P) continue;

            if(Q <= dct[i_img(mat_width, j, i)]) dct[i_img(mat_width, j, i)] += Q;
            else if(-Q >= dct[i_img(mat_width, j, i)]) dct[i_img(mat_width, j, i)] -= Q;
            else {
                bit = get_next_bit(msg, msg_len, msg_index, bit_num);

                //printf("%u %f\n", bit, dct[i_img(mat_width, j, i)]);

                if(!bit) dct[i_img(mat_width, j, i)] *= 2;
                else dct[i_img(mat_width, j, i)] = 2*dct[i_img(mat_width, j, i)] + 1;

                //printf("%u %f\n", bit, dct[i_img(mat_width, j, i)]);

            }

        }
    }
}

static void recover_reversible_pixels(const long double dct[mat_height * mat_width], 
                                      char * restrict msg, uint32_t msg_len, uint32_t * restrict msg_index, 
                                      uint8_t * restrict bit_num)
{
    assert(dct != NULL);
    assert(msg != NULL);
    assert(msg_index != NULL);
    assert(bit_num != NULL);
    
    for(uint8_t j = 0; j < mat_height; j++){
        if(*msg_index >= msg_len) break;

        for(uint8_t i = 0; i < mat_width; i++){
            if (*msg_index >= msg_len) break;
            
            if((i + j) <= P) continue;

            if(2*Q <= dct[i_img(mat_width, j, i)]);
            else if(dct[i_img(mat_width, j, i)] <= -2*Q);
            else{
                
                //printf("%f\n", dct[i_img(mat_width, j, i)]);


                if( ((int32_t)abs(round(dct[i_img(mat_width, j, i)]))) % 2 == 0) 
                    embed_bits_to_msg(msg, msg_index, bit_num, 0, 1, msg_len);
                else
                    embed_bits_to_msg(msg, msg_index, bit_num, 1, 1, msg_len);
            } 

        }
    }
}

// Return values:
// -1 - Small image
// -2 - 0 message
// -3 - Error in greyscale conversion
int8_t reversible_DCT_encrypt(Image* st_img, uint32_t msg_len, const char * restrict msg){
    assert(st_img->img_p != NULL);
    assert(msg != NULL);

    if(msg_len == 0){
        fprintf(stderr, "Error: zero size message provided.\n");
        return -2;
    }
    
    if(st_img->height < 8 || st_img->width < 8){
        fprintf(stderr, "Small image");
        return -1;
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

    uint8_t channels = st_img->channels;
    uint64_t height = st_img->height - (st_img->height % mat_height);
    uint64_t width = (st_img->width - (st_img->width % mat_width)) * channels;

    long double input[mat_width * mat_height];
    long double output[mat_height * mat_width];

    uint32_t msg_index = 0;
    uint8_t bit_num = 0;

    for(uint64_t j = 0; j < height; j += 8){
        if(msg_index >= msg_len) break;

        for(uint64_t i = 0; i < width; i += 8 * channels){
            if(msg_index >= msg_len) break;

            for(uint8_t k = 0; k < mat_height; k++){
                for(uint8_t l = 0; l < mat_width; l++){
                    input[i_img(mat_width, k, l)] = st_img->img_p[i_img(st_img->width*channels, i + l*channels, j + k)];
                }
            }

            transform_subimage(input, output);
            embed_reversible_pixels(output, msg, msg_len, &msg_index, &bit_num);
            invsere_transform_subimage(output, input);

            for(uint8_t k = 0; k < mat_height; k++){
                for(uint8_t l = 0; l < mat_width; l++){
                    st_img->img_p[i_img(st_img->width*channels, i + l*channels, j + k)] = input[i_img(mat_width, k, l)];
                }
            }

        }
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
// -1 - Small image
// -2 - image not greyscale
// NULL character not counted in msg_len
// msg should be zeroed 
int8_t reversible_DCT_decrypt(const Image * restrict st_img, uint32_t msg_len, char * restrict msg){
    assert(st_img->img_p != NULL);
    assert(msg != NULL);
    
    if(msg_len == 0){
        return 0;
    }

    if(st_img->height < 8 || st_img->width < 8){
        fprintf(stderr, "Small image");
        return -1;
    }

    if(st_img->channels > 2) return -2;

    uint8_t channels = st_img->channels;
    uint64_t height = st_img->height - (st_img->height % mat_height);
    uint64_t width = (st_img->width - (st_img->width % mat_width)) * channels;

    long double input[mat_width * mat_height];
    long double output[mat_height * mat_width];

    uint32_t msg_index = 0;
    uint8_t bit_num = 0;


    for(uint64_t j = 0; j < height; j += 8){
        if(msg_index >= msg_len) break;

        for(uint64_t i = 0; i < width; i += 8 * channels){
            if(msg_index >= msg_len) break;

            for(uint8_t k = 0; k < mat_height; k++){
                for(uint8_t l = 0; l < mat_width; l++){
                    input[i_img(mat_width, k, l)] = st_img->img_p[i_img(st_img->width*channels, i + l*channels, j + k)];
                }
            }

            transform_subimage(input, output);
            recover_reversible_pixels(output, msg, msg_len, &msg_index, &bit_num);
        }
    }

    return 0;
}

#undef mat_height
#undef mat_width
#undef P
#undef Q

#endif
