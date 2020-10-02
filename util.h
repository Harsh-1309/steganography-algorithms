#ifndef UTIL
#define UTIL

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

extern bool str_case_cmp(const char * p1, const char * p2);
extern uint8_t get_bit_from_char(uint8_t n, uint8_t c);

#endif