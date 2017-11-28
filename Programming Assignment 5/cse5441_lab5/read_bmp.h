
/*
---------------------------------------------------------------------
---------------------------------------------------------------------
FILE: read_bmp.h
	Contains all the declarations of functionalities supported 
	by BMP Reader library. Include the header file into the project
	to link the functions into your project space
Library: read_bmp.o
---------------------------------------------------------------------
---------------------------------------------------------------------
*/
#ifndef _BMP_READ_LIB_
#define _BMP_READ_LIB_

typedef unsigned int    uint32_t;
typedef signed int      int32_t;
typedef unsigned short  uint16_t;
typedef signed short    int16_t;
typedef unsigned char   uint8_t;
typedef signed char     int8_t;

//----------------------------------------------------				
//FUNC: get_image_width()
//DESCP:
//	Returns the width of image in pixels. Will return 
//  0 if the image is not loaded. Use read_bmp_file 
//  to first load the image
//NOTE:	Will work only on .bmp image files and after 
//      image is loaded using read_bmp_file
//----------------------------------------------------
uint32_t get_image_width();

//----------------------------------------------------				
//FUNC: get_image_height()
//DESCP:
//	Returns the height of image in pixels. Will return 
//  0 if the image is not loaded. Use read_bmp_file 
//  to first load the image
//NOTE:	Will work only on .bmp image files and after 
//      image is loaded using read_bmp_file
//----------------------------------------------------
uint32_t get_image_height();

//----------------------------------------------------				
//FUNC: get_num_pixel()
//DESCP:
//	Returns the total number of pixels on image. Will return 
//  0 if the image is not loaded. Use read_bmp_file 
//  to first load the image
//NOTE:	Will work only on .bmp image files and after 
//      image is loaded using read_bmp_file
//----------------------------------------------------
uint32_t get_num_pixel();

//----------------------------------------------------				
//FUNC: read_bmp_file(FILE *bmp_image_file)
//DESCP:
//	Reads the image file from FILE provided, 
//	Extracts the header information of BMP file
//	Allocates memory with pixel values and returns
//	the data
//RETURN:
//  void * to memory buffer with pixel
//NOTE:	Will work only on .bmp image files
//----------------------------------------------------
void* read_bmp_file(FILE *bmp_file);


//----------------------------------------------------				
//FUNC: write_bmp_file(FILE *out_file, uint8_t *bmp_data)
//DESCP:
//	Writes the image file to FILE * provided, 
//	Adds the header information of BMP file
//	Copies pixel values from buffer provided
//RETURN:
//  NONE
//NOTE:	Will create .bmp type file formats
//----------------------------------------------------
void write_bmp_file(FILE *out_file, uint8_t *bmp_data);

#endif
