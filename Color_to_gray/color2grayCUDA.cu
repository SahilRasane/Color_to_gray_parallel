/*
Convert an image to grayscale.

The code is written for a mini-project of ITCS 5145 Parallel Programming at UNCC.(Q.C.)

To compile the code, we use
        nvcc -arch compute_35 -o color2grayCUDA color2grayCUDA.cu

To run the code, type
        ./color2gray ${input color image} ${output grayscale image} ${image type}

        The format of images depends on its types.
        To specify image type, we have ${image type} as follows:
            1 is for .png file
            2 is for .jpg file
        
        For example,
        ./color2grapy lena1.png lena2.png 1
        ./color2grapy lizard1.jpg lizard2.jpg 2
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

__global__ void colorToGrayscale(unsigned char *gray_img, unsigned char * color_img,int width,int height);

const int IS_PNG = 1;
const int IS_JPG = 2;
const int DESIRED_CHANNELS = 3;
const int MAX_NAME_LENGTH = 500;


int main(int argc, char *argv[]) {
    if (argc < 4){
        printf("Usage: color2Grayscale ${input color image file} ${output grayscale image file} ${image type}\n Image Types:\n\t1: PGN\n\t2: JPG");
	exit(-1);
    }

    int width, height, channels, type;
    char  in_name[MAX_NAME_LENGTH], out_name[MAX_NAME_LENGTH];
    strcpy(in_name, argv[1]);
    strcpy(out_name, argv[2]);
    type = atoi(argv[3]);

    unsigned char *color_img = stbi_load(in_name, &width, &height, &channels, 0); // load and conver the image to 3 channels (ignore the transparancy channel)
    if(color_img == NULL) {
        printf("Error in loading the image\n");
        exit(-1);
    }
    printf("Loaded image %s with a width of %dpx, a height of %dpx and %d channels\n", in_name, width, height, channels);

    // Convert the input image to gray
    int gray_channels = channels == 4 ? 2 : 1;
    size_t gray_img_size = width * height * gray_channels;
    
    unsigned char *gray_img = (unsigned char *)malloc(gray_img_size);
    if(gray_img == NULL) {
        printf("Unable to allocate memory for the gray image.\n");
        exit(1);
    }
    printf("Create a image array with a width of %dpx, a height of %dpx and %d channels\n", width, height, gray_channels);


    unsigned char *d_gray_img ;
    unsigned char *d_color_img;
    
    cudaMalloc((void **)&d_gray_img, sizeof(char)*gray_img_size);
    cudaMalloc((void **)&d_color_img, sizeof(char) * gray_img_size*3);
    
   
    dim3 threadPerblock(16,16);
    dim3 gridSize((width/16)+1,(height/16)+1); 
    
    
	cudaMemcpy(d_gray_img, gray_img,  sizeof(char)*gray_img_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_color_img, color_img, sizeof(char)*gray_img_size*3, cudaMemcpyHostToDevice);

    colorToGrayscale <<<gridSize,threadPerblock>>>(d_gray_img, d_color_img,width,height);

    
	cudaMemcpy(gray_img, d_gray_img, sizeof(char)*gray_img_size, cudaMemcpyDeviceToHost);
    cudaMemcpy(color_img, d_color_img,  sizeof(char)*gray_img_size*3, cudaMemcpyDeviceToHost);
    
    cudaFree(d_color_img);
    cudaFree(d_gray_img);

    if (type == IS_PNG)
    	stbi_write_png(out_name, width, height, gray_channels, gray_img, width * gray_channels);
    else
        if (type == IS_JPG)
            stbi_write_jpg(out_name, width, height, gray_channels, gray_img, 100); //The last parameter of the stbi_write_jpg function is a quality parameter that goes from 1 to 100. Since JPG is a lossy image format, you can chose how much data is dropped at save time. Lower quality means smaller image size on disk and lower visual image quality.
    printf("Wrote image %s with a width of %dpx, a height of %dpx and %d channels\n", out_name, width, height, channels);

    stbi_image_free(gray_img); 
}

__global__ void colorToGrayscale(unsigned char *gray_img, unsigned char * color_img,int width,int height)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;
    
    if (x < width && y < height) 
    {
       
        int grayOffset = y*width + x;
        
        int rgbOffset = grayOffset*DESIRED_CHANNELS;
        unsigned char r = color_img[rgbOffset]; 
        unsigned char g = color_img[rgbOffset + 1]; 
        unsigned char b = color_img[rgbOffset + 2]; 
        
        gray_img[grayOffset] = 0.21f*r + 0.71f*g + 0.07f*b;
    }
    
}
