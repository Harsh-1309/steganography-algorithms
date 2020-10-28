#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "image.h"
#include "constants.h"

#include "algorithms/simple_lsb.h"
#include "algorithms/pvd_greyscale.h"
#include "algorithms/pvd_4px.h"
#include "algorithms/edge_detect_lsb.h"
#include "algorithms/reversible_DCT.h"

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

        /*
        if(str_case_cmp(steg_algo_used, "simple_lsb") == true){
            printf("SIMPLE LSB\n");            
            simple_lsb_encrypt(st_img, strlen(msg), msg);
            
            char output[strlen(img_path) + 1 + 3];
            string_cpy(output, strlen(img_path), img_path);
            append_en_to_image_name(output, strlen(img_path), 's');

            write_png(output, st_img);
        }else 
        */

        Image img = load_image(img_path);
        
        //Image grey = convert_to_greyscale(&img);
        //Integral_Image* ii = create_integral_image(&grey);
        SList* l = interest_points(&img);

        
        Node* nxt;
        Node* cur = l->head;
        for(uint64_t i = 0; i < l->size; i++, cur = nxt){
            nxt = cur->nxt;
            fprintf(stdout, "(%lu, %lu, %u)\n", cur->x, cur->y, cur->l);
        }

        return 0;
        if(str_case_cmp(steg_algo_used, "PVD_greyscale") == true){
            printf("PVD greyscale\n");
            Partitions* par = create_partitions(6, 8, 8, 16, 32, 64, 128);
            e_PVD_GREY eg = construct_e_pvd_grey_struct(img_path, strlen(msg), msg, par);
            pvd_grayscale_encrypt(eg);

            char output[strlen(img_path) + 1 + 3];
            string_cpy(output, strlen(img_path), img_path);
            append_en_to_image_name(output, strlen(img_path), 'p');

            write_png(output, *(eg.st_img));
            destroy_e_pvd_grey_struct(&eg);
            destroy_partitions(par);  
        }else if(str_case_cmp(steg_algo_used, "PVD_4px") == true){
            printf("PVD 4px\n");
            e_PVD4x ep = construct_e_PVD4x_struct(img_path, strlen(msg), msg, 3, 4, 15);
            pvd_4px_encrypt(ep);

            char output[strlen(img_path) + 1 + 3];
            string_cpy(output, strlen(img_path), img_path);
            append_en_to_image_name(output, strlen(img_path), '4');

            write_png(output, *(ep.st_img));
            destroy_e_PVD4x_struct(&ep);

        }else if(str_case_cmp(steg_algo_used, "Edge_LSB") == true){
            printf("Edge_LSB\n");
            e_Edge_Detect ed = construct_e_edge_detect_struct(img_path, strlen(msg), msg, 3, 1, 5);

            edge_detect_encrypt(ed);

            char output[strlen(img_path) + 1 + 3];
            string_cpy(output, strlen(img_path), img_path);
            append_en_to_image_name(output, strlen(img_path), 'e');

            write_png(output, *(ed.st_img));  
            destroy_e_edge_detect_struct(&ed);
        }else if(str_case_cmp(steg_algo_used, "Reversible_DCT") == true){
            printf("Reversible_DCT\n");
            e_rDCT er = construct_e_rdct_struct(img_path, strlen(msg), msg, 1, 4);
            reversible_DCT_encrypt(er);

            char output[strlen(img_path) + 1 + 3];
            string_cpy(output, strlen(img_path), img_path);
            append_en_to_image_name(output, strlen(img_path), 'r');

            write_png(output, *(er.st_img));
            destroy_e_rdct_struct(&er);  
        }

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

        /*
        if(str_case_cmp(steg_algo_used, "simple_lsb") == true){
            char demsg[msg_len + 1];
            simple_lsb_decrypt(st_img, msg_len, demsg);
            printf("%s\n", demsg);
        }else 
        */
        if(str_case_cmp(steg_algo_used, "PVD_greyscale") == true){
            //Paritioning scheme 8, 8, 16, 32, 64, 128
            Partitions* par  = create_partitions(6, 8, 8, 16, 32, 64, 128);
            d_PVD_GREY dg = construct_d_pvd_grey_struct(img_path, msg_len, par);
            pvd_grayscale_decrypt(dg);
            print_buffer(dg.stream);
            destroy_d_pvd_grey_struct(&dg);
            destroy_partitions(par);
        }else if(str_case_cmp(steg_algo_used, "PVD_4px") == true){
            d_PVD4x dp = construct_d_PVD4x_struct(img_path, msg_len, 3, 4, 15);
            pvd_4px_decrypt(dp);
            print_buffer(dp.stream);
            destroy_d_PVD4x_struct(&dp);
        }else if(str_case_cmp(steg_algo_used, "Edge_LSB") == true){
            d_Edge_Detect ed = construct_d_edge_detect_struct(img_path, msg_len, 3, 1, 5);
            edge_detect_decrypt(ed);
            print_buffer(ed.stream);
            destroy_d_edge_detect_struct(&ed);
        }else if(str_case_cmp(steg_algo_used, "Reversible_DCT") == true){
            d_rDCT dr = construct_d_rdct_struct(img_path, msg_len, 1, 4);
            reversible_DCT_decrypt(dr);
            print_buffer(dr.stream);
            destroy_d_rdct_struct(&dr);
        }
    }else{
        fprintf(stderr, "Invalid arguments provided: %s exiting ...\n", argv[1]);
        exit(1);
    }
    return 0;
}