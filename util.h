#ifndef UTIL
#define UTIL

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

typedef struct uint8_pair {
    uint8_t x;
    uint8_t y;
} u8_Pair;

typedef struct uint8_quad {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t w;
} u4_Quad;


extern bool str_case_cmp(const char * p1, const char * p2);
extern uint8_t get_bit_from_char(uint8_t n, uint8_t c);
extern uint8_t bits_to_val(const char* restrict arr, uint8_t num_bits, uint8_t bit_num);
extern void append_en_to_image_name(char* restrict arr, uint32_t msg_len, char c);
extern uint64_t i_img(uint32_t width, uint64_t x, uint64_t y);

#endif