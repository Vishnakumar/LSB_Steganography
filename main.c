/***********************************************************************************
 * 
 * Project     : LSB Steganography (Encoding and Decoding)
 * Author      : Vishnakumar M
 * Description : This program performs image steganography by either encoding
 *               a secret text file into a BMP image or decoding a hidden message
 *               from a stego image back into a text file.
 *
 ***********************************************************************************/

/* Header Files */
#include "types.h"
#include "encode.h"
#include "decode.h"
#include "string.h"


/* Main function */
int main(int argc, char *argv[]){

    // Step 1 : Check the count of the argument, if less than or equal to 3, it will print the error message and finish the program
    if(argc <= 3){
        //printf("ERROR : Argc count is less than or equal to 3\n");
        printf("Usage: %s -e <source.bmp> <secret.txt> [output.bmp]\n", argv[0]);
        printf("   or: %s -d <stego.bmp> [output.txt]\n", argv[0]);
        return 1;
    }

    //Step 2 : Call the function for finding the operation type
    //For finding the  operation type is encoding 
    if(check_operation_type(argv[1]) == e_encode){

        printf("You have selected encoding operation\n");

        //Stpe 2.1 : Declare the structure 
        EncodeInfo enc_info;

        //Step 2.2 Call the read_and_validate_encode_args function, and validate the arguments
        if(read_and_validate_encode_args(argv, &enc_info) == success){

            printf("INFO : Sucessfully read and validate the arguments\n");
            printf("#################### Start the Encoding ####################\n");
       
            //Step 2.2.1 Call do_encoding function
            if(do_encoding(&enc_info) == success){
                printf("############# Encoding Successfully Completed #############\n");
                return 0;
            }
            else{
                printf("ERROR : Encoding is not sucessfully completed\n");
                return 1;
            }
        }
    }
    //For finding the  operation type is Decoding
    else if (check_operation_type(argv[1]) == e_decode){
        
        printf("You have selected decoding operation\n");
        printf("#################### Start the Decoding ####################\n");

        // Step 2.1 : Declare structure variable
        DecodeInfo dec_info;

        // Step 2.2 : call the read_and_validate_encode_args function, and validate the arguments
        if(read_and_validate_decode_args(argv, &dec_info) == success){

            printf("INFO : Sucessfully read and validate the arguments\n");

            //Step 2.2.3 : call do_encoding function
            if(do_decoding(&dec_info) == success){
                printf("############# Decoding Successfully Completed #############\n");
                return 0;
            }
            else{
                printf("ERROR : Decoding is not sucessfully completed\n");
                return 1;
            }
        }
    }

    else{
        //Or print the error message in terminal
        printf("ERROR : Unsupported operation.\n Please use -e or -d\n");
        return 1;
    }    
}


/* This function is used to find the operation type */
OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol, "-e") == 0){
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0){
        return e_decode;
    }
    else{
        return e_unsupported;
    }
}



