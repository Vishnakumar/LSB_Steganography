#ifndef DECODE_H
#define DECODE_H

/* Header Files */
#include "encode.h"
#include "stdio.h"
#include "types.h"

// Decode Info structure
typedef struct DecodeInfo
{
    /* Stego image section */
    char *stego_image_fname;
    FILE *fptr_stego_image;


    /* Secret file names*/
    char *secret_fname;
    FILE *fptr_secret;
    int extn_size;
    char extn_secret_file[11];
    long size_secret_file;

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode Secret File Extn size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);


/* Decode Secret File Extn */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode Secret File Size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode Secret File Data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from LSB of image data */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decode size from LSB of image data */
Status decode_size_from_lsb(int *data, char *image_buffer);

#endif