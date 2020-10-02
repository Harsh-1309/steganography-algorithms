#include "util.h"

#include <string.h>
#include <ctype.h>
#include <tgmath.h>
#include "constants.h"

bool str_case_cmp(const char * p1, const char * p2){
    for(unsigned int i = 0; p1[i] != '\0'; i++){
        if(isalpha(p1[i])){
            if(tolower(p1[i]) != tolower(p2[i])) return false;
            else                                 continue;
        }

        if(p1[i] != p2[i]) return false;
    }

    return true;
}

uint8_t get_bit_from_char(uint8_t n, uint8_t c){
    assert(n < 8);

    uint8_t mask = 1;
    mask <<= n;

    return ((c & mask) >> n);
}

uint8_t bits_to_val(const char* restrict arr, uint8_t num_bits, uint8_t bit_num){
    assert(bit_num <= 7);
    assert(num_bits <= 7);
    assert(arr != NULL);

    uint32_t len = strlen(arr);
    uint8_t sum = 0;
    for(uint8_t i = 0; i < num_bits && i != num_bits; i++){
        if((i + bit_num)/NUM_BITS_IN_CHAR >= len)
            break;
        sum += get_bit_from_char((i + bit_num) % NUM_BITS_IN_CHAR, arr[(i + bit_num)/NUM_BITS_IN_CHAR]) * pow(2, i);
    }

    return sum;
}

//Not counting null character in msg_len
void append_en_to_image_name(char* restrict arr, uint32_t msg_len, char c){
    uint32_t i = msg_len - 1;
    for(; i != (uint32_t)(-1); i--){
        if(arr[i] == '/')
            break;
    }

    if(i == (uint32_t)(-1))
        i = 0;
    
    for(uint32_t j = msg_len; j != i; j--){
        arr[j + 3] = arr[j];
    }
    arr[i + 1] = 'e';
    arr[i + 2] = c;
    arr[i + 3] = '_';
}

uint64_t i_img(uint32_t width, uint64_t x, uint64_t y){
    return x + y*width;
}

uint8_t min(uint8_t a, uint8_t b){
    return a > b ? b : a;
}

uint8_t min_4(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
    return min(min(a, b), min(c, d));
}


uint8_t max(uint8_t a, uint8_t b){
    return a > b ? a : b;
}
uint8_t max_4(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
    return max(max(a, b), max(c, d));
}