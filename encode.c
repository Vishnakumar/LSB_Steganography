//Header files
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Validate the source file */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo){

    //Step 1 : Check the extension of source file is .bmp or not
    char *result = strstr(argv[2], ".bmp");
    if(result != NULL && strcmp(result, ".bmp") == 0){

        encInfo->src_image_fname = argv[2];

        //Step 2 : Check the extension of secret file, it might be .h, .c, .sh and .txt
        result = strstr(argv[3], ".");

        if (result != NULL && (strcmp(result, ".h") == 0 || strcmp(result, ".c") == 0 || strcmp(result, ".sh") == 0 || strcmp(result, ".txt") == 0)){
            
            //If valid, store the argv[3] into structure
            encInfo->secret_fname = argv[3];

            //Step 3 : Check the extension of output file is .bmp, this file don't need to exist, if not exist and create default one .bmp file
            if(argv[4] != NULL){

                result = strstr(argv[4], ".bmp");
                
                if(result != NULL && strcmp(result, ".bmp") == 0){

                    //If valid, store the argv[4] into structure
                    encInfo->stego_image_fname = argv[4];
                    return success;
                }
                else{
                    printf("ERROR : Output file is not a .bmp file\n");
                    return failure;
                }
            }
            else {
                encInfo->stego_image_fname = "default.bmp";
                return success;
            }
        }
        else{
            printf("ERROR : Unsupported secret file format\n");
            return failure;
        }
    } 
    else{
        printf("ERROR : Source file is not a .bmp file\n");
        return failure;
    }

    return 1;

}

/* Open the files */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");

    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->src_image_fname);
        return failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->secret_fname);

        return failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
   
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->stego_image_fname);

        return failure;
    }

    // No failure return success
    return success;
}

/* Get the size of the image */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("Width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("Height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* Get the size of the file */
uint get_file_size(FILE *fptr)
{
    
    if(fptr == NULL){
        printf("ERROR : File doesn't open\n");
        return 0;
    }

    //Move the file pointer to end of the file
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

/*
 This Function is used to check the source file's enough to store the secret file
*/
Status check_capacity(EncodeInfo *encInfo){

    //Step 1 : Get the size of the image
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    //Get the file size of the secret file
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    //Total bytes required for secret + metadata
    uint total_required_bytes = (strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(int) + encInfo->size_secret_file) * 8;

    if(encInfo->image_capacity > total_required_bytes){
        return success;
    }
    else{
        return failure;
    }
}

/*  Following function is used to copy the header in source bmp file to destination bmp file, 
    header have 54 bytes, so we have to copy that 54 bytes of data. Because they should not be changed at all*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image){

    //Declare the buffer with size of 54
    char buffer[54];

    //Move the file pointer back to beginning of the source file 
    rewind(fptr_src_image);

    //Step 1 : Read the header's data from source file into buffer
    if(fread(buffer, 54, 1, fptr_src_image) != 1){
        perror("ERROR : Reading the header content from file\n");
        return failure;
    };

    //Step 2 : write the data in buffer into destination file
    if(fwrite(buffer, 54, 1, fptr_dest_image) != 1){
        perror("ERROR : Writing the header content from file\n");
        return failure;
    }

    //Step 3 : Check the copy is done or not
    //if both value is 54, it indicates we have done it, otherwise we haven't done
    if(ftell(fptr_src_image) == ftell(fptr_dest_image)){
        return success;
    }
    else{
        return failure;
    }
}


/* Encode the magic string into the destinatino file */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo){

    //Declaration of buffer with size of 8 
    char buffer[8];

    //Run the loop based on the length of the magic string
    for(int i = 0 ; i < strlen(magic_string) ; i++){

        // Read 8 bytes from source image file
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1){
            printf("ERROR: Unable to read from source image while encoding magic string\n");
            return failure;
        }

        //encode the magic string
        encode_byte_to_lsb(magic_string[i], buffer);

        // Write the modified bytes to the stego image file
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1){
            printf("ERROR : Unable to write to stego image while encoding magic string\n");
            return failure;
        }
    }
    return success;
}


/* Following is used to find the size of the extension data*/
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo){

    //Declaration
    char buffer[32];

    //Read the 32 bytes from the source image
    if(fread(buffer, 32, 1, encInfo->fptr_src_image) != 1){
        perror("ERROR : Read the data from source file\n");
        return failure;
    }

    //Perform the encode operation
    encode_size_to_lsb(size, buffer);

    //Write the encoded 32 bytes of data into the destination file
    if(fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1){
        perror("ERROR : Writing the data into the destination file\n");
        return failure;
    }

    //Return
    return success;
}

/* Encode the size of the secret file */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo){

    // Declaration of the buffer with size of 8
    char buffer[8];

    //Run the loop upto the size of the file
    for(size_t i=0;i < strlen(file_extn) ; i++){

        //Read the data from the source file
        if(fread(buffer,8,1, encInfo -> fptr_src_image) != 1){
            perror("ERROR : Read the data from source file\n");
            return failure;
        }

        //Perform th encode operation
        encode_byte_to_lsb(file_extn[i], buffer );

        //Write the data into the destination file
        if(fwrite(buffer,8,1, encInfo -> fptr_stego_image) != 1){
            perror("ERROR : Read the data from source file\n");
            return failure;
        }
    }

    //return
    return success;
}

/* Encode the size of the secret file like 2 or 3 or 4 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo){
    
    //Declaration of the character array
    char buffer[32];

    //Read the 32 bytes of data from source image 
    if(fread(buffer, 32, 1, encInfo->fptr_src_image) != 1){
        perror("ERROR : Read the data from source file\n");
        return failure;
    }

    //Encode the file size into that 32 bytes of data
    encode_size_to_lsb(file_size, buffer);

    //Write the 32 bytes of encoded data into stego image
    if(fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1){
        
        perror("ERROR : Writing the data to the destination file\n");
        return failure;
    }

    //Return the sucess image
    return success;

}

/* Encode the extension name of the secret file data in the destination file */
Status encode_secret_file_data(EncodeInfo *encInfo){

    //Move the file pointer back to beginning of the source file 
    rewind(encInfo -> fptr_secret);

    // Read the bytes based of size of secret file and store into the 
    fread(encInfo -> secret_data , encInfo ->size_secret_file, 1, encInfo -> fptr_secret);

    // Declaration of buffer with size of 8
    char buffer[8];

    // Run the loop based on size in the secret file
    for(size_t i=0; i < encInfo -> size_secret_file; i++){

            // Read the 8 bytes data from the source file
            if(fread(buffer,8,1, encInfo -> fptr_src_image) != 1){
                perror("ERROR : Read the data from source file\n");
                return failure;
            }

            // Perform the encode operation
            encode_byte_to_lsb(encInfo -> secret_data[i] , buffer );

            // Write the encoded 8 bytes of data into the destination file
            if(fwrite(buffer,8,1, encInfo -> fptr_stego_image) != 1){
                perror("ERROR : Write the data into the destination file\n");
                return failure;
            }
    }

    //Return
    return success;
}


/* Copy the remaining data in the souce image */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    //Declaration
    char ch;

    //Read the content by one byte from source, then write that byte to destination file
    while (fread(&ch, 1, 1, fptr_src) == 1){
        if(fwrite(&ch, 1, 1, fptr_dest) != 1){
            perror("ERROR : Writing to destination file\n");
            return failure;
        }
    }

    //Check if loop was ended while reading the data byte by byte from source file
    if(ferror(fptr_src)){
        perror("ERROR : Reading from source file\n");
        return failure;
    }

    //Otherwise return the sucess messages
    return success;
}


/* Encode the Bytes in the source image */
/* For Data */
Status encode_byte_to_lsb(char data, char *image_buffer){

    // Performing the encode operation
    for(int i = 0 ; i < 8 ; i++ ){
        image_buffer[i] = (image_buffer[i] & ~1) | ((data >> i) & 1 );
    }

}

/* For size */
Status encode_size_to_lsb(int size, char *imageBuffer){

    // Performing the encode operation
    for(int i = 0 ; i < 32 ; i++ ){
        imageBuffer[i] = (imageBuffer[i] & ~1) | ((size >> i) & 1 );
    }

}

/* Following function perform the encoding operation by calling required function one by one */
Status do_encoding(EncodeInfo *encInfo){

    //Step 1 : Open the required files
    if(open_files(encInfo) == success){
        printf("INFO : Files are opened sucessfully\n");
    }
    else{
        printf("ERROR : Unable to open the file\n");
        return failure;
    }

    //Step 2 : Check the capacity of the image
    if(check_capacity(encInfo) == success){
        printf("INFO : Image has enough capacity to encode the secret data into it\n");
    }
    else{
        printf("ERROR : Image doesn't has enough capacity to hold the data\n");
        return failure;
    }

    //Step 3 : Copy the Header content of the source file to destination file
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == success){
        printf("INFO : Sucessfully copied the header content from source file to destination file\n");
    }
    else{
        printf("ERROR : Unable to copy the content from source file to destination file\n");
        return failure;
    }

    //Step 4 : Encode the magic string
    if(encode_magic_string(MAGIC_STRING, encInfo) == success){
        printf("INFO : Sucessfully encode the magic string into the destional file\n");
    }
    else{
        printf("ERROR : Unable to encode the magic string into the file\n");
        return failure;
    }

    //Step 5 : Find the extension size
    //Declaration
    char *ptr = strchr(encInfo->secret_fname, '.');
    if(encode_secret_file_extn_size(strlen(ptr), encInfo) == success){
        printf("INFO : Sucessfully encode the size of the extension name\n");
    }
    else{
        printf("ERROR : Unable to encode the size of the extension name\n");
    }

    //Step 6 : Call the function for encode the name of the file extension
    if(encode_secret_file_extn(ptr, encInfo) == success){
        printf("INFO : Sucessfully encode the extension name\n");
    }
    else{
        printf("ERROR : Unable to encode the extension name\n");
        return failure;
    }

    // Step 7: Encode secret file size
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == success){
        printf("INFO : Size of the Secret file is successfully encoded\n");
    }
    else{
        printf("ERROR : Unable to encode the size of the secret file\n");
        return failure;
    }

    // Step 8: Encode secret file data
    if (encode_secret_file_data(encInfo) == success){
        printf("INFO : Data in secret file is sucessfully encoded\n");
    }
    else{
        printf("ERROR : Unable to encode the secret file's data\n");
        return failure;
    }

    // Step 9: Copy remaining image bytes
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == success){
        printf("INFO : Remaining image data copied successfully\n");
    }
    else{
        printf("ERROR : Failed to copy the remaining data from the image\n");
        return failure;
    }

    //Return the sucess
    return success;
}

