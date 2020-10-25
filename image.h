#ifndef IMAGE
#define IMAGE

#include <stdint.h>
#include <stdbool.h>

#include "util.h"

enum image_creator{
    MALLOC, 
    LIBRARY
};

typedef struct integral_image{
    uint64_t width;
    uint64_t height;
    uint64_t image_size;
    uint64_t integral[];
} Integral_Image;

typedef struct{
    uint64_t width;
    uint64_t height;
    int8_t channels;
    uint8_t* img_p;
    uint64_t image_size;
    enum image_creator ic;
} Image;

extern Image load_image(const char* img_path);
extern void write_png(const char* img_path, Image img);
extern void free_image(Image* img);
//color:
//0 - r, 1 - g, 2 - b, 3 - a
//grey:
//0 - g, 1 - a
extern uint8_t get_pixel(const Image* img, uint64_t x, uint64_t y, uint8_t channel);
extern void set_pixel(Image* img, uint64_t x, uint64_t y, uint8_t channel, uint8_t value);

extern Image convert_to_greyscale(const Image* img);
extern Image fuzzy_edge_detector(const Image* img);
extern Image canny_edge_detector(const Image* img);
extern Image hybrid_edge_detector(const Image* img);
extern Integral_Image* create_integral_image(const Image * img);
extern void destroy_integral_image(Integral_Image * ii_img);
extern long double * det_of_hessian(const Image * img, uint8_t octave, uint8_t index);
extern Slist * interest_points(const * Image img);


#endif