#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "image.h"

#include "algorithms/simple_lsb.h"
#include "algorithms/pvd_greyscale.h"

int main(int argc, char** argv){
    if (argc < 2){ 
        fprintf(stderr, "Insufficient number of arguments provided exiting ...\n");
        exit(1);
    }

    if(str_case_cmp(argv[1], "-e") == true){
        if(argc < 4){
           fprintf(stderr, "Insufficient number of arguments provided exiting ...\n");
           exit(1);
        }

        const char* steg_algo_used = argv[2];
        const char* msg = argv[3];
        const char* img_path = argv[4];

        printf("Image path: %s\n", img_path);

        Image st_img = load_image(img_path);

        if(str_case_cmp(steg_algo_used, "simple_lsb") == true){
            printf("SIMPLE LSB\n");
            simple_lsb_encrypt(st_img, strlen(msg), msg);
            write_png("encrypted_simple_lsb.png", st_img);
        }else if(str_case_cmp(steg_algo_used, "PVD_greyscale") == true){
            printf("PVD greyscale\n");
            pvd_grayscale_encrypt(&st_img, strlen(msg), msg);
            write_png("en_pvd_sky.png", st_img);  
        }

        free_image(&st_img);
    }else if(str_case_cmp(argv[1], "-d") == true){
        if(argc < 4){
           fprintf(stderr, "Insufficient number of arguments provided exiting ...\n");
           exit(1);
        }

        const char* steg_algo_used = argv[2];
        uint32_t msg_len = strtol(argv[3], NULL, 10);
        const char* img_path = argv[4];

        printf("Image path: %s\n", img_path);
        printf("Message len: %d\n", msg_len);

        Image st_img = load_image(img_path);

        if(str_case_cmp(steg_algo_used, "simple_lsb") == true){
            char demsg[msg_len + 1];
            simple_lsb_decrypt(st_img, msg_len, demsg);
            printf("%s\n", demsg);
        }else if(str_case_cmp(steg_algo_used, "PVD_greyscale") == true){
            char demsg[msg_len + 1];
            for(int i =0; i<=msg_len;i++) demsg[i] = '\0';
            pvd_grayscale_decrypt(&st_img, msg_len, demsg);
            printf("%s\n", demsg);
        }
        free_image(&st_img);
    }else{
        fprintf(stderr, "Invalid arguments provided: %s exiting ...\n", argv[1]);
        exit(1);
    }
    return 0;
}