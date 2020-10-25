#include <stdint.h>
#include <stdlib.h>
#include <tgmath.h>
#include <stdbool.h>
#include <assert.h>

#include "util.h"
#include "image.h"
#include "constants.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"




Image load_image(const char * img_path){
    Image i = {};
    int32_t width, height, channels;

    i.img_p = stbi_load(img_path, &width, &height, &channels, 0);

    if(width < 0 || height < 0 || channels < 0)
        return (Image){0, 0, 0, NULL, 0, LIBRARY};

    i.width = width;
    i.height = height;
    i.channels = channels;
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

uint8_t get_pixel(const Image* img, uint64_t x, uint64_t y, uint8_t channel){
    if(channel >= img->channels) return 0;
    if(x >= img->width || y >= img->height) return 0;

    return img->img_p[i_img(img->width * img->channels, x * img->channels, y) + channel];
}

extern void set_pixel(Image* img, uint64_t x, uint64_t y, uint8_t channel, uint8_t value){
    if(channel >= img->channels) return;
    if(x >= img->width || y >= img->height) return;

    img->img_p[i_img(img->width * img->channels, x * img->channels, y) + channel] = value;
}

Image convert_to_greyscale(const Image* img){
    Image grey = {img->width, img->height, img->channels == 4 ? 2 : 1, NULL, 0, MALLOC}; 
    grey.image_size = grey.width * grey.height * grey.channels;
    grey.img_p = malloc(grey.image_size * sizeof(uint8_t));
    
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

Image fuzzy_edge_detector(const Image* img){
    if(img->channels > 2) return (Image){0};

    const uint8_t W = 3;
    const uint8_t TAU = 4;
    const float P = 1.0f;

    //Finding the max grey value
    uint8_t max_value = 0;
    for(uint64_t i = 0; i < img->image_size; i += img->channels){
        if(img->img_p[i] > max_value){
            max_value = img->img_p[i];
            if (max_value == 255) break;
        } 
    }

    //Normalisation with WxW neighbour
    float * const fnorm_img = malloc(img->image_size/img->channels * sizeof(float));
    
    if(fnorm_img == NULL){
        fprintf(stderr, "Failed to allocate memory for the normalisation.\n");
        return (Image){0};
    } 

    const uint64_t width = img->width * img->channels;
    uint8_t min_g = 255;
    uint8_t max_g = 0;
    const uint8_t gi = img->channels - 1;
    uint8_t g = 0; 
    
    const uint8_t w_half = W/2;
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
    float * const fedge_img = malloc(img->image_size/img->channels * sizeof(float));
    
    if(fedge_img == NULL){
        fprintf(stderr, "Failed to allocate memory for the edginess.\n");
        return (Image){0};
    }

    float sum = 0.0f;
    float f = 0.0f;
    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < img->width; i++){
            
            for(int8_t k = -w_half; k <= w_half; k++){
                for(int8_t l = -w_half; l <= w_half; l++){
                    if((i + l != -1) && (j + k != -1) && (i + l < width) && (j + k < img->height)){
                        f = fnorm_img[i_img(img->width, i + l, j + k)];
                        sum += fmin(f, pow(1 - pow(f, P), 1.0/P));
                    }
                }
            }

            fedge_img[i_img(img->width, i, j)] = fmin(1.0f, (float)TAU/W * sum);
            sum = 0.0f;
        }
    }
    
    free(fnorm_img);

    Image edge = {img->width, img->height, img->channels, NULL, img->image_size, MALLOC};
    edge.img_p = malloc(edge.image_size);
    
    if(edge.img_p == NULL){
        fprintf(stderr, "Failed to allocate memory for the greyscale conversion.\n");
        return (Image){0};
    }

    
    for(uint64_t i = 0; i < edge.image_size; i += edge.channels){
        edge.img_p[i] = fedge_img[i/edge.channels] == 1 ? 255 : 0 ;
        if(edge.channels == 2)
             edge.img_p[i + 1] = img->img_p[i + 1]; 
    }

    free(fedge_img);
    return edge;
}

static uint8_t sort_angle(float angle){
    if(angle < 0)
        angle += M_PI;


    //angle 0
    if ((0 <= angle && angle < M_PI/8.0) || (7.0*M_PI/8.0 <= angle && M_PI))
        return 0;
    //angle 45
    else if ((M_PI/8.0 <= angle && angle < 3.0*M_PI/8.0))
        return 1;
    //angle 90
    else if ((3.0*M_PI/8.0 <= angle && angle < 5.0*M_PI/8.0))
        return 2;
    //angle 135
    else if ((5.0*M_PI/8.0 <= angle && angle < 7.0*M_PI/8.0))
        return 3;

    assert(0);
}

Image canny_edge_detector(const Image* img){
    if(img->channels > 2) return (Image){0};

    const float gaussian_5x5[25] = {0.003765, 0.015019, 0.023792, 0.015019, 0.003765, 
                                    0.015019, 0.059912,	0.094907, 0.059912, 0.015019,
                                    0.023792, 0.094907,	0.150342, 0.094907,	0.023792,
                                    0.015019, 0.059912,	0.094907, 0.059912,	0.015019,
                                    0.003765, 0.015019, 0.023792, 0.015019,	0.003765};
    
    const int8_t sobel_x[9] = {-1, 0, 1,
                              -2, 0, 2,
                              -1, 0, 1};
    
    const int8_t sobel_y[9] = {1, 2, 1,
                               0, 0, 0,
                               -1, -2, -1};
     
    //Applying gaussian blur
    const uint8_t blur_w = 2;
    const uint8_t gi = img->channels - 1;
    const uint64_t width = img->width * img->channels;

    Image blur = {img->width, img->height, img->channels, NULL, img->image_size, MALLOC};
    blur.img_p = malloc(blur.image_size);

    float sum = 0.0f;
    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < width; i += img->channels){
            if(blur.channels == 2)
                blur.img_p[i + 1] = img->img_p[i + 1]; 

            for(int8_t k = -blur_w; k <= blur_w; k++){
                for(int8_t l = -blur_w; l <= blur_w; l++){
                    if((i + l*(gi + 1) != -1) && (j + k != -1) && (i + l*(gi + 1) < width) && (j + k < img->height)){
                        sum += img->img_p[i_img(width, i + l*(gi + 1), j + k)] * gaussian_5x5[i_img(5, blur_w + l, blur_w + k)];
                    }
                }
            }

            blur.img_p[i_img(width, i, j)] = u8_fclamp(sum);
            sum = 0.0f;
        }
    }

    //Sobel operator
    float * const grad_mag = malloc(blur.image_size/blur.channels * sizeof(float));
    if(grad_mag == NULL){
        fprintf(stderr, "Failed to allocate memory for the gradient intensity.\n");
        return (Image){0};
    }

    uint8_t * const angle = malloc(blur.image_size/blur.channels * sizeof(uint8_t));
    if(angle == NULL){
        fprintf(stderr, "Failed to allocate memory for the gradient angle.\n");
        return (Image){0};
    }

    float i_x = 0.0f;
    float i_y = 0.0f;
    const uint8_t sobel_w = 1;
    float g_max = 0.0f;
    for(uint64_t j = 0; j < blur.height; j++){
        for(uint64_t i = 0; i < width; i += blur.channels){

            for(int8_t k = -sobel_w; k <= sobel_w; k++){
                for(int8_t l = -sobel_w; l <= sobel_w; l++){
                    if((i + l*(gi + 1) != -1) && (j + k != -1) && (i + l*(gi + 1) < width) && (j + k < blur.height)){
                        i_x += blur.img_p[i_img(width, i + l*(gi + 1), j + k)] * sobel_x[i_img(3, sobel_w + l, sobel_w + k)];
                        i_y += blur.img_p[i_img(width, i + l*(gi + 1), j + k)] * sobel_y[i_img(3, sobel_w + l, sobel_w + k)];
                    }
                }
            }

            grad_mag[i_img(blur.width, i, j)] = hypot(i_x, i_y);
            if(hypot(i_x, i_y) > g_max){
                g_max = hypot(i_x, i_y);
            }
            angle[i_img(blur.width, i, j)] = sort_angle(atan2(i_y, i_x));
            i_y = i_x = 0.0f;
        }
    }

    for(uint64_t i = 0; i < blur.image_size; i += blur.channels){
        blur.img_p[i] = u8_fclamp(grad_mag[i/blur.channels]/g_max * 255.0);
    }

    free(grad_mag);
    
    //Non-maximum suppression
    uint8_t p = 0;
    uint8_t n = 0;
    uint8_t max_pixel = 0;
    for(uint64_t j = 1; j < blur.height - 1; j++){
        for(uint64_t i = 1; i < width - 1; i += blur.channels){

            switch(angle[i_img(blur.width, i, j)]){
                case 0:
                    {
                        p = blur.img_p[i_img(width, i - 1*(gi + 1), j)];
                        n = blur.img_p[i_img(width, i + 1*(gi + 1), j)];
                    }
                    break;
                case 1:
                    {
                        p = blur.img_p[i_img(width, i + 1*(gi + 1), j - 1)];
                        n = blur.img_p[i_img(width, i - 1*(gi + 1), j + 1)];
                    }
                    break;
                case 2:
                    {
                        p = blur.img_p[i_img(width, i, j - 1)];
                        n = blur.img_p[i_img(width, i, j + 1)];
                    }   
                    break;
                case 3:
                    {
                        p = blur.img_p[i_img(width, i - 1*(gi + 1), j - 1)];
                        n = blur.img_p[i_img(width, i + 1*(gi + 1), j + 1)];
                    } 
                    break;
                default:
                    assert(0);
            }

            if(blur.img_p[i_img(width, i, j)] < p || blur.img_p[i_img(width, i, j)] < n){
                blur.img_p[i_img(width, i, j)] = 0;
                continue;
            }

            if(blur.img_p[i_img(width, i, j)] > max_pixel) max_pixel = blur.img_p[i_img(width, i, j)];
        }
    }

    //Setting edge pixels to 0
    for(uint64_t i = 0; i < width; i += blur.channels){
        blur.img_p[i_img(width, i, 0)] = blur.img_p[i_img(width, i, blur.height - 1)] = 0;
    }

    for(uint64_t i = 0; i < blur.height; i += 1){
        blur.img_p[i_img(width, 0, i)] = blur.img_p[i_img(width, width - 1 - gi, i)] = 0;
    }


    free(angle);

    //Double thresholding
    const float high_threshold = 0.09f * max_pixel;
    const float low_threshold = 0.05f * high_threshold;

    const uint8_t strong_pixel = 255;
    const uint8_t weak_pixel = 25;
    uint8_t cur_pixel = 0.0f;

    for(uint64_t j = 0; j < blur.height; j++){
        for(uint64_t i = 0; i < width; i += blur.channels){
            cur_pixel = blur.img_p[i_img(width, i, j)];

            if(cur_pixel < low_threshold)
                blur.img_p[i_img(width, i, j)] = 0;
            else if(cur_pixel >= low_threshold && cur_pixel < high_threshold){
                blur.img_p[i_img(width, i, j)] = weak_pixel;
            }else if(cur_pixel >= high_threshold){
                blur.img_p[i_img(width, i, j)] = strong_pixel;
            }else{
                assert(0);
            }

        }
    }    


    Image output = {img->width, img->height, img->channels, NULL, img->image_size, MALLOC};
    output.img_p = malloc(blur.image_size);

    //Edge tracking with hytersis
    bool has_strong_pixel = false;
    for(uint64_t j = 0; j < blur.height; j++){
        for(uint64_t i = 0; i < width; i += blur.channels){
            if(blur.img_p[i_img(width, i, j)] == strong_pixel || blur.img_p[i_img(width, i, j)] == 0){
                output.img_p[i_img(width, i, j)] = blur.img_p[i_img(width, i, j)];
                continue;
            }
                

            for(int8_t k = -sobel_w; k <= sobel_w; k++){
                for(int8_t l = -sobel_w; l <= sobel_w; l++){
                    if((i + l*(gi + 1) != -1) && (j + k != -1) && (i + l*(gi + 1) < width) && (j + k < blur.height)){
                        if(blur.img_p[i_img(width, i + l*(gi + 1), j + k)] == strong_pixel){
                            has_strong_pixel = true;
                            break;
                        }
                    }
                }

                if(has_strong_pixel) break;
            }

            if(has_strong_pixel) output.img_p[i_img(width, i, j)] = strong_pixel;
            else output.img_p[i_img(width, i, j)] = 0;

            has_strong_pixel = false;
        }
    }

    free_image(&blur);

    return output;
}

Image hybrid_edge_detector(const Image* img){
    if(img->channels > 2)
        return (Image){0};

    Image fuzzy_img = fuzzy_edge_detector(img);
    Image canny_img = canny_edge_detector(img);



    for(uint64_t i = 0; i < img->image_size; i += img->channels){
        if(fuzzy_img.img_p[i] == 255){
            canny_img.img_p[i] = 255;
        }
    }

    free_image(&fuzzy_img);
    return canny_img;
}

/*
static float highf_function(float f){
    float z_square = f*f;
    float z_cube = f*f*f;

    return (1 - 9/z_square + 16/z_cube - 9/(z_square * z_square) + 1/(z_cube * z_cube))/power_2(4);
}

static float lowf_function(float f){
    float z_square = f*f;
    float z_cube = f*f*f;
    float z_five = f*f*f*f*f;
    float z_seven = f*f*f*f*f*f*f;

    return (- 1 + 14/z_square - 16/z_cube - 31/(z_square * z_square) + 80/z_five + 164/(z_cube * z_cube) 
            + 80/z_seven - 31/(z_square * z_square * z_square * z_square) - 16/(z_cube * z_cube * z_cube) 
            + 14/(z_five * z_five) - 1/(z_cube * z_cube * z_cube * z_cube))/power_2(8);
}

Image LL_filter(const Image* img){
    if(img->channels > 2) break;

    Image output = {img->width, img->height, img->channels, NULL, img->image_size, MALLOC};
    output.img_p = malloc(blur.image_size);

    for(uint64_t i = 0; i < img->image_size; i += img->channels){

    }

}

Image highpass_filter(const Image* img){
    if(img->channels > 2) break;

    Image output = {img->width, img->height, img->channels, NULL, img->image_size, MALLOC};
    output.img_p = malloc(blur.image_size);

    for(uint64_t i = 0; i < img->image_size; i += img->channels){
        
    }

}*/

static uint64_t sum_image(const Image * img, uint64_t w, uint64_t h){
    uint64_t sum = 0;
    for(uint64_t j = 0; j <= h; j++){
        for(uint64_t i = 0; i <= w; i++){
            sum += img->img_p[i_img(img->width * img->channels, i * img->channels, j)];
        }
    }

    return sum;
}

//Image can be symmetrically extended.
Integral_Image* create_integral_image(const Image * img){
    if(img->img_p == NULL){
        fprintf(stderr, "Null image provided.\n");
        return NULL;
    }

    if(img->channels > 2){
        fprintf(stderr, "Not a greyscale image.\n");
        return NULL;
    }

    Integral_Image * ii_img = malloc(sizeof(Integral_Image) + sizeof(uint64_t[img->width * img->height]));
    if(ii_img == NULL){
        fprintf(stderr, "Error in allocating space for integral image.\n");
        return NULL;
    }

    ii_img->width = img->width;
    ii_img->height = img->height;
    ii_img->image_size = img->width * img->height;

    for(uint64_t j = 0; j < ii_img->height; j++){
        for(uint64_t i = 0; i < ii_img->width; i++){
            ii_img->integral[i_img(ii_img->width, i, j)] = sum_image(img, i, j);
        }
    }

    return ii_img;
}

void destroy_integral_image(Integral_Image * ii_img){
    assert(ii_img != NULL);
    free(ii_img);
}


/*//b, d are +ve
//a, c are -ve
static uint64_t box_filter(const Integral_Image * ii_img, int8_t a, int8_t b, int8_t c, int8_t d, 
                           uint64_t i, uint64_t j){
    
    uint64_t u1, u2, u3, u4;
    if((i + a) >= ii_img->width || (j + c) >= ii_img->height){
        u1 = 0;
    }else{
        u1 = ii_img->integral[i_img(ii_img->width, i + a, j + c)];
    }

    if((i + a) >= ii_img->width || j < (d + 1)){
        u2 = 0;
    }else{
        u2 = ii_img->integral[i_img(ii_img->width, i + a, j - d - 1)];
    }

    if(i < (b + 1) || j < (d + 1)){
        u3 = 0;
    }else{
        u3 = ii_img->integral[i_img(ii_img->width, i - b - 1, j - d - 1)];
    }

    if(i < (b + 1) || (j + c) >= ii_img->height){
        u4 = 0;
    }else{
        u4 = ii_img->integral[i_img(ii_img->width, i - b - 1, j + c)];
    }

    return u1 - u2 + u3 - u4;
}

static int32_t Dyy(const Image * img, uint64_t i, uint64_t j, uint8_t scale, int8_t* dyy_filer){
    /*uint64_t b1 = box_filter(ii_img, l-1, l-1, (3*l-1)/2, (3*l-1)/2, i, j);
    uint64_t b2 = box_filter(ii_img, l-1, l-1, (l-1)/2, (l-1)/2, i, j);

    //assert((b1 - 3*b2) < 256);
    if(b1 < 3*b2) return 0;
    else if(b1 - 3*b2 > 255) return 255;

    return (uint8_t)(b1 - 3*b2);
}*/

static int32_t convolution(const Image * img, uint64_t i, uint64_t j, uint8_t scale, uint8_t width,
                           int8_t* filter, uint8_t w_half, uint8_t h_half){
    int32_t val = 0;

    
    for(int16_t k = -h_half; k <= h_half; k++){
        for(int16_t l = -w_half; l <= w_half; l++){

            if(k < 0){
                if(j < k * (-1)) continue;
            }else if(k > 0){
                if(j + k >= img->height) continue;
            }

            if(l < 0){
                if(i < l*(-1)) continue;
            }else if(l > 0){
                if(i + l >= img->width) continue;
            }

            val += (int16_t)get_pixel(img, i + l, j + k, 0) * filter[i_img(width, l + w_half, k + h_half)];

        }        
    }  
    
    return val;
}

static int8_t* get_Dyy_filter(uint8_t scale){
    uint8_t width = 2 * scale - 1;
    uint8_t height = 3 * scale;
    int8_t * buf = malloc(width * height);

    for(uint8_t j = 0; j < height; j++){
        for(uint8_t i = 0; i < width; i++){
            if(j >= scale && j < 2 * scale) buf[i_img(width, i, j)] = -2;
            else buf[i_img(width, i, j)] = 1;
        }
    }

    return buf;
}

static int8_t* get_Dxx_filter(uint8_t scale){
    uint8_t width = 3 * scale;
    uint8_t height = 2 * scale - 1;
    int8_t * buf = malloc(width * height);

    for(uint8_t j = 0; j < height; j++){
        for(uint8_t i = 0; i < width; i++){
            if(i >= scale && i < 2 * scale) buf[i_img(width, i, j)] = -2;
            else buf[i_img(width, i, j)] = 1;
        }
    }

    return buf;
}

static int8_t* get_Dxy_filter(uint8_t scale){
    uint8_t width = 2 * scale + 1;
    uint8_t height = 2 * scale + 1;
    int8_t * buf = malloc(width * height);

    for(uint8_t j = 0; j < height; j++){
        for(uint8_t i = 0; i < width; i++){
            if ((i >= 0 && i < scale) && (j >= 0 && j < scale)) buf[i_img(width, i, j)] = -1;
            else if ((i >= scale + 1 && i < width) && (j >= 0 && j < scale)) buf[i_img(width, i, j)] = +1;
            else if ((i >= 0 && i < scale) && (j >= scale + 1 && j < height)) buf[i_img(width, i, j)] = +1;
            else if ((i >= scale + 1 && i < width) && (j >= scale + 1 && j < height)) buf[i_img(width, i, j)] = -1;
            else buf[i_img(width, i, j)] = 0;
        }
    }

    return buf;    
}

long double * det_of_hessian(const Image * img, uint8_t scale){    
    assert(img != NULL);

    if(3 * scale > img->width || 3 * scale > img->height){
        fprintf(stderr, "Image too small.\n");
        return NULL;
    }

    long double* det = malloc(img->image_size * sizeof(long double));

    int8_t * dxx = get_Dxx_filter(scale);
    uint8_t dxx_width = 3 * scale;
    uint8_t dxx_w_half = (3 * scale - 1)/2;
    uint8_t dxx_h_half = scale - 1;
    int32_t dxx_val = 0;



    int8_t * dyy = get_Dyy_filter(scale);
    uint8_t dyy_width = 2 * scale - 1;
    uint8_t dyy_w_half = scale - 1;
    uint8_t dyy_h_half = (3 * scale - 1)/2;
    int32_t dyy_val = 0;



    int8_t * dxy = get_Dxy_filter(scale);
    uint8_t dxy_width = 2 * scale + 1;
    uint8_t dxy_w_half = scale;
    uint8_t dxy_h_half = scale;
    int32_t dxy_val = 0;

    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < img->width; i++){

            dxy_val = convolution(img, i, j, scale, dxy_width, dxy, dxy_w_half, dxy_h_half);
            dxx_val = convolution(img, i, j, scale, dxx_width, dxx, dxx_w_half, dxx_h_half);
            dyy_val = convolution(img, i, j, scale, dyy_width, dyy, dyy_w_half, dyy_h_half);

            det[i_img(img->width, i, j)] = 1.0/(scale * scale * scale * scale) * ((long double)dxx_val * dyy_val 
                                                                                 - (0.912 * dxy_val) * (0.912 * dxy_val));
        }
    }

    free(dyy);
    free(dxy);
    free(dxx);

    return det;
}   

static bool is_maximum_3x3x3(uint64_t width, uint64_t height, const long double* dets[3], 
                             uint64_t x, uint64_t y){

    long double center = dets[1][i_img(width, x, y)];
    for(int8_t k = 0; k < 3; k++){
        for(int8_t j = -1; j <= 1; j++){
            for(int8_t i = -1; i <= 1; i++){
                if(k == 1 && i == 0 && j = 0) continue;
                if((x == 0 && i == -1) || (x + i >= width) || (y == 0 && j == -1) || (y + j >= height)) continue;

                if(dets[k][i_img(width, x + i, y + j)] > center) return false;

            }
        }
    }

    return true;
}

static key_points(uint8_t ocatave, uint8_t index, uint64_t threshold, 
                  uint64_t width, uint64_t height, 
                  const long double* dets[3], Slist* sl){
    
    for(uint64_t j = 0; j < width; j++){
        for(uint64_t i = 0; i < height; i++){
            if(dets[1][i_img(width, i, j)] <= threshold) continue;
            if(!is_maximum_3x3x3(width, height, dets, i, j)) continue;
            
        }
    }

}

Slist * interest_points(const * Image img){
    const uint8_t max_octave = 4;
    const uint8_t max_index = 4;

    const uint8_t max_scales = 10;
    const uint8_t scales[max_scales] = {3, 5, 7, 9, 13, 17, 25, 33, 49, 65};

    //calculate dets
    const long double* dets[10]; 
    for(uint8_t i = 0; i < max_scales; i++) dets[i] = det_of_hessian(img, scales[i]);
 
    Slist* key_point_list = create_linked_list();

    //Add key_points


    //Free the scales
    for(uint8_t i = 0; i < max_scales; i++) free(dets[i]);

    return Slist;
}
