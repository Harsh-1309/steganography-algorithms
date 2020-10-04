#ifndef UTIL
#define UTIL

#include <stdint.h>
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
} u8_Quad;


extern bool str_case_cmp(const char * p1, const char * p2);
extern uint8_t get_bit_from_char(uint8_t n, uint8_t c);
extern uint8_t bits_to_val(const char* restrict arr, uint8_t num_bits, uint32_t len,
                           uint8_t * restrict bit_num, uint32_t * restrict msg_index);
extern void append_en_to_image_name(char* restrict arr, uint32_t msg_len, char c);
extern uint64_t i_img(uint32_t width, uint64_t x, uint64_t y);
extern uint8_t min(uint8_t a, uint8_t b);
extern uint8_t min_4(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
extern uint8_t max(uint8_t a, uint8_t b);
extern uint8_t max_4(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
extern uint8_t power_2(uint8_t k);
extern uint8_t u8_fclamp(float f);

#endif