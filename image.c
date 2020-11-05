#include <stdint.h>
#include <stdlib.h>
#include <tgmath.h>
#include <stdbool.h>
#include <assert.h>
#include <float.h>

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

    return i;
}

void write_png(const char* img_path, Image img){
    stbi_write_png(img_path, img.width, img.height, img.channels, img.img_p, img.width*img.channels);
}

Image create_empty_image(uint64_t width, uint64_t height, uint8_t channels){
    Image img = {width, height, channels, NULL, width * height * channels, MALLOC};
    img.img_p = malloc(img.image_size);
    if(img.img_p == NULL){
        fprintf(stderr, "Failed to create the image.\n");
    }

    return img;
}

void free_image(Image* img){
    if(img->img_p == NULL) return;

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

void set_pixel(Image* img, uint64_t x, uint64_t y, uint8_t channel, uint8_t value){
    if(channel >= img->channels) return;
    if(x >= img->width || y >= img->height) return;

    img->img_p[i_img(img->width * img->channels, x * img->channels, y) + channel] = value;
}

uint64_t get_ipixel(const Integral_Image* img, uint64_t x, uint64_t y){
    if(x >= img->width || y >= img->height) return 0;

    return img->integral[i_img(img->width, x, y)];
}

void set_ipixel(Integral_Image* img, uint64_t x, uint64_t y, uint64_t value){
    if(x >= img->width || y >= img->height) return;

    img->integral[i_img(img->width, x, y)] = value;
}

Image convert_to_greyscale(const Image* img){
    if(img->channels <= 2){
        fprintf(stderr, "Image already greyscale.\n");
        return (Image) {0, 0, 0, NULL, 0, MALLOC};
    }

    Image grey = {img->width, img->height, img->channels == 4 ? 2 : 1, NULL, 0, MALLOC}; 
    grey.image_size = grey.width * grey.height * grey.channels;
    grey.img_p = malloc(grey.image_size * sizeof(uint8_t));
    
    if(grey.img_p == NULL){
        fprintf(stderr, "Failed to allocate memory for the greyscale conversion.\n");
        return (Image){0};
    }

    for(uint64_t i = 0, j = 0; i < img->image_size && j < grey.image_size; i += img->channels, j += grey.channels){
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

    set_ipixel(ii_img, 0, 0, get_pixel(img, 0, 0, 0));

    for(uint64_t i = 1; i < ii_img->width; i++){
        set_ipixel(ii_img, i, 0, get_pixel(img, i, 0, 0) + get_ipixel(ii_img, i - 1, 0));    
    }

    uint64_t s;
    for(uint64_t j = 1; j < ii_img->height; j++){
        s = get_pixel(img, 0, j, 0);
        set_ipixel(ii_img, 0, j, get_ipixel(ii_img, 0, j - 1) + s);
        for(uint64_t i = 1; i < ii_img->width; i++){
            s += get_pixel(img, i, j, 0);
            set_ipixel(ii_img, i, j, get_ipixel(ii_img, i, j - 1) + s);
        }
    }

    return ii_img;
}

void destroy_integral_image(Integral_Image * ii_img){
    assert(ii_img != NULL);
    free(ii_img);
}

static uint64_t box_filter(const Integral_Image * ii_img, int8_t a, int8_t b, int8_t c, int8_t d, 
                           uint64_t i, uint64_t j){
    
    uint64_t u1, u2, u3, u4;
    uint64_t right, bottom, left, top;
    
    const uint64_t width = ii_img->width;
    const uint64_t height = ii_img->height;

    if(b > 0 && (i + b) >= width) right = width - 1;
    else if(b < 0 && i < b * (-1)) right = 0; 
    else right = i + b;

    if(d > 0 && (j + d) >= height) bottom = height - 1;
    else if (d < 0 && j < d * (-1)) bottom = 0;
    else bottom = j + d;

    if(a > 0 && (i + a) >= width) left = width - 1;
    else if (a < 0 && i < a * (-1)) left = 0;
    else left = i + a;

    if(c > 0 && (j + c) >= height) top = height - 1;
    else if (c < 0 && j < c * (-1)) top = 0;
    else top = j + c;

    if(top == 0 && bottom == 0 && left == 0 && right == 0) return 0;

    u1 = get_ipixel(ii_img, right, bottom);
    if(top == 0) u2 = 0; 
    else u2 = get_ipixel(ii_img, right, top - 1);
    
    if(left == 0) u3 = 0;
    else u3 = get_ipixel(ii_img, left - 1, bottom);

    if(left == 0 || top == 0) u4 = 0;
    else u4 = get_ipixel(ii_img, left - 1, top - 1);

    return u1 - u2 - u3 + u4;
}

static int32_t Dyy(const Integral_Image * ii_img, uint64_t i, uint64_t j, uint8_t scale){
    uint64_t b1 = box_filter(ii_img, -(scale - 1), scale - 1, -(3 * scale - 1)/2, (3 * scale - 1)/2, i, j);
    uint64_t b2 = box_filter(ii_img, -(scale - 1), scale - 1, -(scale - 1)/2, (scale - 1)/2, i, j);

    return b1 - (int32_t)3*b2;
}

static int32_t Dxx(const Integral_Image* ii_img, uint64_t i, uint64_t j, uint8_t scale){
    uint64_t b1 = box_filter(ii_img, -(3 * scale - 1)/2, (3 * scale - 1)/2, -(scale - 1), scale - 1, i, j);
    uint64_t b2 = box_filter(ii_img, -(scale - 1)/2, (scale - 1)/2, -(scale - 1), scale - 1, i, j);

    return b1 - (int32_t)3*b2;
}

static int32_t Dxy(const Integral_Image* ii_img, uint64_t i, uint64_t j, uint8_t scale){
    uint64_t b1 = box_filter(ii_img, 1, scale, -scale, -1, i, j);
    uint64_t b2 = box_filter(ii_img, -scale, -1, -scale, -1, i, j);
    uint64_t b3 = box_filter(ii_img, -scale, -1, 1, scale, i, j);
    uint64_t b4 = box_filter(ii_img, 1, scale, 1, scale, i, j);


    return (b1 - (int32_t)b2) + (b3 - (int32_t)b4);
}

/*
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
*/

typedef struct doh {
    long double * buf;
    uint64_t width;
    uint64_t height;
    uint8_t scale;
} DoH;

static DoH * det_of_hessian(const Integral_Image * img, uint8_t scale){    
    assert(img != NULL);

    if(3 * scale > img->width || 3 * scale > img->height){
        fprintf(stderr, "Image too small.\n");
        return NULL;
    }

    long double* det = malloc(img->width * img->height * sizeof(long double));
    if(det == NULL){
        fprintf(stderr, "Unable to allocate memory.\n");
        return NULL;
    }

    int32_t dxx_val = 0;
    int32_t dyy_val = 0;
    int32_t dxy_val = 0;

    for(uint64_t j = 0; j < img->height; j++){
        for(uint64_t i = 0; i < img->width; i++){

            dxy_val = Dxy(img, i, j, scale);
            dxx_val = Dxx(img, i, j, scale);
            dyy_val = Dyy(img, i, j, scale);

            det[i_img(img->width, i, j)] = 1.0/(scale * scale * scale * scale) * ((long double)dxx_val * dyy_val 
                                                                                 - (0.912 * dxy_val) * (0.912 * dxy_val));
        }
    }

    DoH * d = malloc(sizeof(DoH));
    if(d == NULL){
        fprintf(stderr, "Unable to allocate memory for DoH.\n");
        return NULL;
    }

    d->width = img->width;
    d->height = img->height;
    d->scale = scale;
    d->buf = det;

    return d;
}

static void free_doh(DoH * d){
    if(d == NULL || d->buf == NULL) return;
    free(d->buf);
    free(d);
}

static bool is_maximum_3x3x3(const DoH* dets[3], uint64_t x, uint64_t y){

    uint64_t width = dets[1]->width;
    uint64_t height = dets[1]->height;
    long double center = dets[1]->buf[i_img(width, x, y)];
    for(int8_t k = 0; k < 3; k++){
        for(int8_t j = -1; j <= 1; j++){
            for(int8_t i = -1; i <= 1; i++){
                if(k == 1 && i == 0 && j == 0) continue;
                if((x == 0 && i == -1) || (x + i >= width) || (y == 0 && j == -1) || (y + j >= height)) continue;

                if(dets[k]->buf[i_img(width, x + i, y + j)] > center) return false;

            }
        }
    }

    return true;
}

static void key_points(uint64_t threshold, const DoH* dets[3], SList* sl, uint64_t w, bool* seleted_pixels){
    
    uint64_t width = dets[1]->width;
    uint64_t height = dets[1]->height;
    uint8_t scale = dets[1]->scale;

    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < width; i++){
            if(dets[1]->buf[i_img(width, i, j)] <= threshold) continue;
            if(!is_maximum_3x3x3(dets, i, j)) continue;

            if(!seleted_pixels[i_img(w, i, j)]){
                append_list(sl, create_node(i, j, scale));
                seleted_pixels[i_img(w, i, j)] = true;
            }
        }
    }

}

SList * interest_points(const Image * img, int32_t threshold){
#define max_octave 4
#define max_index 4
#define max_scales 10

    uint8_t scales[max_scales] = {3, 5, 7, 9, 13, 17, 25, 33, 49, 65};
    uint8_t ocatves[max_octave][max_index] = {
        {0, 1, 2, 3},
        {1, 3, 4, 5},
        {3, 5, 6, 7},
        {5, 7, 8, 9}
    };

    //calculate dets
    DoH* dets[10];
    Integral_Image* ii = create_integral_image(img); 
    uint8_t octaves_filled = 0;
    uint8_t scales_used;
    bool is_completely_filled = false;

    for(scales_used = 0; scales_used < max_scales; scales_used++){
        dets[scales_used] = det_of_hessian(ii, scales[scales_used]);

        if(dets[scales_used] == NULL){
            for(uint8_t k = 0; k < max_octave; k++){
                for(uint8_t j = 0; j < max_index; j++){
                    if (scales[ocatves[k][j]] == scales[scales_used]){
                        octaves_filled = k;
                        is_completely_filled = true;
                        break;
                    }  
                }

                if (is_completely_filled) break;
            }

            break;
        }
    } 

    if(!is_completely_filled) octaves_filled = max_octave;
    destroy_integral_image(ii);

    SList* key_point_list = create_linked_list();

    bool * seleted_pixels = calloc(img->width * img->height, sizeof(bool));
    if(seleted_pixels == NULL){
        fprintf(stderr, "Unable to allocate space for boolean array.\n");
        return NULL;
    }

    //Add key_points
    for(uint8_t j = 0; j < octaves_filled; j++){
        for(uint8_t i = 1; i <= 2; i++){
            key_points(threshold,  
                (const DoH *[3]){
                    dets[ocatves[j][i - 1]],
                    dets[ocatves[j][i]],
                    dets[ocatves[j][i + 1]]
                },
            key_point_list, img->width, seleted_pixels);
        }
    }

    free(seleted_pixels);
    //Free the scales
    for(uint8_t i = 0; i < scales_used; i++) free_doh(dets[i]);

    return key_point_list;

#undef max_octave
#undef max_index
#undef max_scales
}

static void discrete_convolution(uint8_t filter_size,  const long double* restrict filter, 
                                 uint64_t signal_size, const long double* restrict signal,  
                                 uint64_t output_size, long double * restrict output){
    long double sum = 0.0;

    uint64_t kmin, kmax;
    for(uint64_t n = 0; n < output_size; n++){
        kmin = (n >= filter_size - 1) ? (n - (filter_size - 1)) : 0;
        kmax = (n < signal_size - 1)  ? n : signal_size - 1;
        
        for(uint64_t k = kmin; k <= kmax; k++){
            sum += signal[k] * filter[n - k];
        }

        output[n] = sum;
        sum = 0.0;
    }

}

static void downsample2(uint8_t filter_size, uint64_t signal_size, const long double * restrict signal, 
                        uint64_t output_size, long double* restrict output){
    for(uint64_t i = filter_size - 1, j = 0; i < signal_size && j < output_size; i += 2, j++){
        output[j] = signal[i];
    }
}

static void upsample2(uint64_t signal_size, const long double * restrict signal, 
                      uint64_t output_size, long double* restrict output){
    
    for(uint64_t i = 0, j = 0; i < signal_size && (j + 1) < output_size; i++, j += 2){
        output[j] = signal[i];
        output[j + 1] = 0.0;
    }
}



static void DWT_rows(uint8_t hp_size, const long double * restrict hf,   
                     uint8_t lp_size, const long double * restrict lf,
                     uint64_t width, uint64_t height, long double* img){

    long double* row = malloc(sizeof(long double) * width);
    if(row == NULL){
        fprintf(stderr, "row NULL\n");
        return;
    }
    
    uint64_t hout_size = hp_size + width - 1;
    uint64_t lout_size = lp_size + width - 1;
    
    long double* hout = malloc(sizeof(long double) * hout_size);
    if(hout == NULL){
        fprintf(stderr, "hout NULL\n");
        return;
    }

    long double* lout = malloc(sizeof(long double) * lout_size);
    if(lout == NULL){
        fprintf(stderr, "lout NULL\n");
        return;
    }

    long double* half = malloc(sizeof(long double) * (width/2));
    if(half == NULL){
        fprintf(stderr, "half NULL\n");
        return;
    }

    uint64_t i;
    uint64_t half_width = width/2;
    for(uint64_t j = 0; j < height; j++){
        for(i = 0; i < width; i++){
            row[i] = img[i_img(width, i, j)];
        }
        discrete_convolution(lp_size, lf, width, row, lout_size, lout);
        downsample2(lp_size, lout_size, lout, half_width, half);


        for(i = 0; i < half_width; i++){
            img[i_img(width, i, j)] = half[i];
        }

        discrete_convolution(hp_size, hf, width, row, hout_size, hout);
        downsample2(hp_size, hout_size, hout, half_width, half);

        for(i = half_width; i < width; i++){
            img[i_img(width, i, j)] = half[i - half_width];
        }
    }

    free(row);
    free(lout);
    free(hout);
    free(half);
}

static void DWT_columns(uint8_t hp_size, const long double * restrict hf,   
                     uint8_t lp_size, const long double * restrict lf,
                     uint64_t width, uint64_t height, long double* img){

    long double* col = malloc(sizeof(long double) * height);
    if(col == NULL){
        fprintf(stderr, "col NULL\n");
        return;
    }

    uint64_t hout_size = hp_size + height - 1;
    uint64_t lout_size = lp_size + height - 1;
    long double* hout = malloc(sizeof(long double) * hout_size);
    if(hout == NULL){
        fprintf(stderr, "hout NULL\n");
        return;
    }

    long double* lout = malloc(sizeof(long double) * lout_size);
    if(lout == NULL){
        fprintf(stderr, "lout NULL\n");
        return;
    }

    long double* half = malloc(sizeof(long double) * (height/2));
    if(half == NULL){
        fprintf(stderr, "half NULL\n");
        return;
    }

    uint64_t i;
    uint64_t half_height = height/2;
    for(uint64_t j = 0; j < width; j++){
        for(i = 0; i < height; i++){
            col[i] = img[i_img(width, j, i)];
        }
        discrete_convolution(lp_size, lf, height, col, lout_size, lout);
        downsample2(lp_size, lout_size, lout, half_height, half);


        for(i = 0; i < half_height; i++){
            img[i_img(width, j, i)] = half[i];
        }

        discrete_convolution(hp_size, hf, height, col, hout_size, hout);
        downsample2(hp_size, hout_size, hout, half_height, half);

        for(i = half_height; i < height; i++){
            img[i_img(width, j, i)] = half[i - half_height];
        }
    }

    free(col);
    free(lout);
    free(hout);
    free(half);
}

long double* cdf_9_7(const Image* img){
    if(img->channels > 2){
        fprintf(stderr, "Input is not a greyscale image.\n");
        return NULL;
    }

    const long double low_pass_filter[] = {
        +0.026748757411, -0.016864118443, -0.078223266529, 
        +0.266864118443, +0.602949018236, +0.266864118443, 
        -0.078223266529, -0.016864118443, +0.026748757411
    };

    const long double high_pass_filter[] = {
        +0.000000000000, -0.091271763114, -0.057543526229, 
        +0.591271763114, -1.115087050000, +0.591271763114, 
        +0.057543526229, -0.091271763114, +0.000000000000
    };
    const uint8_t high_filter_size = 9;
    const uint8_t low_filter_size = 9;
    const uint64_t height = largest_power_2(img->height);
    const uint64_t width = largest_power_2(img->width);

    long double * buf = malloc(sizeof(long double) * width * height);
    if(buf == NULL){
        fprintf(stderr, "buf NULL.\n");
        return NULL;
    }

    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < width; i++){
            buf[i_img(width, i, j)] = get_pixel(img, i, j, 0);
        }
    }

    DWT_rows(high_filter_size, high_pass_filter, low_filter_size, low_pass_filter, width, height, buf);
    DWT_columns(high_filter_size, high_pass_filter, low_filter_size, low_pass_filter, width, height, buf);

    /*long double max = -LDBL_MAX;
    long double min = LDBL_MAX;

    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < width; i++){
            if(buf[i_img(width, i, j)] > max) max = buf[i_img(width, i, j)];
            if (buf[i_img(width, i, j)] < min) min = buf[i_img(width, i, j)];
        }
    }

    Image output_img = create_empty_image(width, height, img->channels);
    
    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < width; i++){
            set_pixel(&output_img, i, j, 0, (buf[i_img(width, i, j)] - min)/(max - min) * 255);
            if(output_img.channels == 2){
                set_pixel(&output_img, i, j, 1, 255);
            }
        }
    }*/

    return buf;
}

static void iDWT_rows(uint8_t hp_size, const long double * restrict hf,   
                     uint8_t lp_size, const long double * restrict lf,
                     uint64_t width, uint64_t height, long double* img){
    
    uint64_t half_width = width/2;
    long double* half1 = malloc(sizeof(long double) * half_width); 
    if(half1 == NULL) return;

    long double* half2 = malloc(sizeof(long double) * half_width);
    if(half2 == NULL) return;

    long double* full1 = malloc(sizeof(long double) * width);
    if(full1 == NULL) return;

    long double* full2 = malloc(sizeof(long double) * width);
    if(full2 == NULL) return;

    uint64_t hout_size = hp_size + width - 1;
    uint64_t lout_size = lp_size + width - 1;
    long double* hout = malloc(sizeof(long double) * hout_size);
    if(hout == NULL) return;
    long double* lout = malloc(sizeof(long double) * lout_size);
    if(lout == NULL) return;

    
    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < half_width; i++){
            half1[i] = img[i_img(width, i, j)];
        }

        for(uint64_t i = half_width; i < width; i++){
            half2[i] = img[i_img(width, i, j)];
        }

        upsample2(half_width, half1, width, full1);
        upsample2(half_width, half2, width, full2);

        discrete_convolution(lp_size, lf, width, full1, lout_size, lout);
        discrete_convolution(hp_size, hf, width, full2, hout_size, hout);

        for(uint64_t i = 0; i < width; i++){
            img[i_img(width, i, j)] = (hout[i] + lout[i]);
        }
    }


    free(half1);
    free(half2);
    free(full1);
    free(full2);
    free(hout);
    free(lout);
}

static void iDWT_columns(uint8_t hp_size, const long double * restrict hf,   
                         uint8_t lp_size, const long double * restrict lf,
                         uint64_t width, uint64_t height, long double* img){

    long double* half1 = malloc(sizeof(long double) * height/2);     if(half1 == NULL) return;
    long double* half2 = malloc(sizeof(long double) * height/2); if(half2 == NULL) return;
    long double* full1 = malloc(sizeof(long double) * height);if(full1 == NULL) return;
    long double* full2 = malloc(sizeof(long double) * height);if(full2 == NULL) return;

    uint64_t hout_size = hp_size + height - 1;
    uint64_t lout_size = lp_size + height - 1;
    long double* hout = malloc(sizeof(long double) * hout_size);if(hout == NULL) return;
    long double* lout = malloc(sizeof(long double) * lout_size);if(lout == NULL) return;
    

    uint64_t half_height = height/2;
    for(uint64_t i = 0; i < width; i++){
        for(uint64_t j = 0; j < half_height; j++){
            half1[j] = img[i_img(width, i, j)];
        }

        for(uint64_t j = half_height; j < height; j++){
            half2[j] = img[i_img(width, i, j)];
        }

        upsample2(half_height, half1, height, full1);
        upsample2(half_height, half2, height, full2);

        discrete_convolution(lp_size, lf, height, full1, lout_size, lout);
        discrete_convolution(hp_size, hf, height, full2, hout_size, hout);

        for(uint64_t j = 0; j < height; j++){
            img[i_img(width, i, j)] = (hout[j] + lout[j]);
        }
    }


    free(half1);
    free(half2);
    free(full1);
    free(full2);
    free(hout);
    free(lout);
}


Image icdf_9_7(long double * buf, uint64_t width, uint64_t height){
    const long double ilow_pass_filter[] = {
        +0.000000000000, +0.091271763114, -0.057543526229, -0.591271763114, 
        -1.115087050000, -0.591271763114, +0.057543526229, +0.091271763114, 
        +0.000000000000
    };

    const long double ihigh_pass_filter[] = {
        +0.026748757411, +0.016864118443, -0.078223266529, -0.266864118443, 
        +0.602949018236, -0.266864118443, -0.078223266529, +0.016864118443, 
        +0.026748757411
    };
    const uint8_t high_filter_size = 9;
    const uint8_t low_filter_size = 9;    

    iDWT_rows(high_filter_size, ihigh_pass_filter, low_filter_size, ilow_pass_filter, width, height, buf);
    iDWT_columns(high_filter_size, ihigh_pass_filter, low_filter_size, ilow_pass_filter, width, height, buf);

    long double max = -LDBL_MAX;
    long double min = LDBL_MAX;

    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < width; i++){
            if(buf[i_img(width, i, j)] > max) max = buf[i_img(width, i, j)];
            if (buf[i_img(width, i, j)] < min) min = buf[i_img(width, i, j)];
        }
    }

    Image output_img = create_empty_image(width, height, 1);
    
    for(uint64_t j = 0; j < height; j++){
        for(uint64_t i = 0; i < width; i++){
            set_pixel(&output_img, i, j, 0, (buf[i_img(width, i, j)] - min)/(max - min) * 255);
            if(output_img.channels == 2){
                set_pixel(&output_img, i, j, 1, 255);
            }
        }
    }

    return output_img;
}