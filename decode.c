// Header files
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Check if the input file is a .bmp file
    char *result = strstr(argv[2], ".bmp");
    if (result != NULL && strcmp(result, ".bmp") == 0){
        
        //If valid, store the name of the stego image into structure
        decInfo->stego_image_fname = argv[2];

        // Optional output file argument, if not equal to null, store the file name into structure
        if (argv[3] != NULL){
            decInfo->secret_fname = argv[3];
        }
        // Otherwise keep it as null
        else{
            decInfo->secret_fname = NULL;
        }

        //Return the sucess
        return success;
    }
    else{
        //Print the error messages
        printf("ERROR : Input file is not a .bmp file\n");
        return failure;
    }
}

/* Open files for decoding */
Status open_decode_files(DecodeInfo *decInfo)
{
    //Open the file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    //If it stego file pointer points the null, then show the error messages
    if (decInfo->fptr_stego_image == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->stego_image_fname);
        return failure;
    }

    //if not equal to null, return the success
    return success;
}

/* Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo){

    //Declaration
    char buffer[8];
    char decoded_char;

    //Declaration of char array with size of magic string plus 1
    char decoded_magic_string[strlen(magic_string) + 1]; 

    // Move to 54th byte (image data starts here)
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // Read the 8 bytes of characters one by one from stego image
    for (int i = 0; i < strlen(magic_string) ; i++)
    {
        //While reading, if error is occured, then return failure
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1){
            return failure;
        }

        // Decode one character
        decode_byte_from_lsb(&decoded_char, buffer);

        // load the decoded char into array
        decoded_magic_string[i] = decoded_char;
    }

    // Add the null character at the end of the array
    decoded_magic_string[strlen(magic_string)] = '\0';

    // Check if the decoded string matches the expected magic string
    if (strcmp(decoded_magic_string, magic_string) != 0){
        printf("ERROR : This file doesn't contain any secret data (not a stego file)\n");
        return failure;
    }

    //Return the success
    return success;
}

/* Decode Secret File Extension Size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo){

    // Declaration of the character array
    char buffer[32];
    int size;

    // Read the 32 bytes of characters from stego image
    //While reading, if error is occured, then return failure
    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1){
        return failure;
    }

    // Decode the extension size
    decode_size_from_lsb(&size, buffer);

    // Validate the size
    if (size <= 0 || size > 4)
    {
        printf("ERROR : Invalid secret file extension size: %d\n", size);
        return failure;
    }

    // Load the size into structure
    decInfo->extn_size = size;

    return success;
}

/* Decode Secret File Extension */
Status decode_secret_file_extn(DecodeInfo *decInfo){

    //Declaration of the buffer
    char buffer[8];
    char decoded_char;

    // Read the 8 bytes of characters from stego image
    for (int i = 0; i < decInfo->extn_size; i++){
        
        //While reading, if error is occured, then return failure
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1){
            return failure;
        }

        // Perform the decode operation
        decode_byte_from_lsb(&decoded_char, buffer);

        // Load the character into array
        decInfo->extn_secret_file[i] = decoded_char;
    }

    // Add the null at end of the string
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    // Set default secret file name if not provided
    if (decInfo->secret_fname == NULL){
        static char default_name[50];
        sprintf(default_name, "decoded_file%s", decInfo->extn_secret_file);
        decInfo->secret_fname = default_name;
    }

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    if (decInfo->fptr_secret == NULL){
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s for writing\n", decInfo->secret_fname);
        return failure;
    }

    return success;
}

/* Decode Secret File Size */
Status decode_secret_file_size(DecodeInfo *decInfo){

    //Declaration
    char buffer[32];
    int file_size;

    // Read the 32 bytes of characters from stego image and While reading, if error is occured, then return failure
    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1){
        return failure;
    }

    // Perform the decode operation
    decode_size_from_lsb(&file_size, buffer);
    decInfo->size_secret_file = file_size;

    // Validate the size of the file
    if (decInfo->size_secret_file <= 0){

        //Print the error message
        printf("ERROR : Decoded secret file size is invalid");
        return failure;
    }

    return success;
}

/* Decode Secret File Data */
Status decode_secret_file_data(DecodeInfo *decInfo){

    //Declaration
    char buffer[8];
    char decoded_char;

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        //Read the 8 bytes of data from stego image
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1){
            return failure;
        }

        //Perform the decode operation
        decode_byte_from_lsb(&decoded_char, buffer);

        //Write the 1 bytes of data into secret file
        fwrite(&decoded_char, 1, 1, decInfo->fptr_secret);
    }

    return success;
}

/* Decode a byte from LSB */
Status decode_byte_from_lsb(char *data, char *image_buffer){

    //Initialization
    *data = 0;

    // Performing the decode operation
    for (int i = 0; i < 8; i++){
        *data |= ((image_buffer[i] & 1) << i);
    }
    return success;
}

/* Decode size (4 bytes / 32 bits) from LSB */
Status decode_size_from_lsb(int *data, char *image_buffer){

    //Initialization
    *data = 0;

    //Performing the decode operation
    for (int i = 0; i < 32; i++){
        *data |= ((image_buffer[i] & 1) << i);
    }
    return success;
}

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    // Step 1 : Open the file
    if (open_decode_files(decInfo) != success){
        printf("ERROR : Unable to open the stego file\n");
        return failure;
    }
    printf("INFO : Files opened successfully\n");

    // Step 2 : Decode magic string
    if (decode_magic_string(MAGIC_STRING, decInfo) != success){
        printf("ERROR: Magic string mismatch\n");
        return failure;
    }
    printf("INFO : Magic string decoded successfully\n");

    // Step 3 : Decode secret file extension size
    if (decode_secret_file_extn_size(decInfo) != success){
        printf("ERROR: Failed to decode extension size\n");
        return failure;
    }
    printf("INFO : Secret file extension size decoded successfully\n");

    // Step 4 : Decode secret file extension
    if (decode_secret_file_extn(decInfo) != success){
        printf("ERROR : Failed to decode file extension\n");
        return failure;
    }
    printf("INFO : Secret file extension decoded successfully\n");

    // Step 5 : Decode secret file size
    if (decode_secret_file_size(decInfo) != success){
        printf("ERROR : Failed to decode secret file size\n");
        return failure;
    }
    printf("INFO : Secret file size decoded successfully\n");

    // Step 6 : Decode secret file data
    if (decode_secret_file_data(decInfo) != success){
        printf("ERROR: Failed to decode secret file data\n");
        return failure;
    }
    printf("INFO : Secret file data successfully extracted to %s\n", decInfo->secret_fname);

    // Step 7 : Close all opened files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);

    // Return the sucesss

    return success;
}
