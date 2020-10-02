#include "util.h"

#include <string.h>
#include <ctype.h>

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