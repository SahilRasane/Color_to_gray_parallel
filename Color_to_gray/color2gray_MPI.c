/*
Convert an image to grayscale.

The code is written for a mini-project of ITCS 5145 Parallel Programming at UNCC.(Q.C.)

To compile the code, we use
         mpicc -g -Wall -o color2grayMPI stb_image/stb_image.h stb_image/stb_image_write.h color2grayMPI.c -lm
To run the code, type
        ./color2grayMPI ${input color image} ${output grayscale image} ${image type}

        The format of images depends on its types.
        To specify image type, we have ${image type} as follows:
            1 is for .png file
            2 is for .jpg file
        
        For example,
        mpiexec -n 4 color2grayMPI lena1.png lena2.png 1
       mpiexec -n 4 color2grayMPI lizard1.jpg lizard2.jpg 2
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

void colorToGrayscale(unsigned char *gray_img, unsigned char * color_img, int element_per_proc);

const int IS_PNG = 1;
const int IS_JPG = 2;
const int DESIRED_CHANNELS = 3;
const int MAX_NAME_LENGTH = 500;
int my_rank, process_count;
int main(int argc, char *argv[]) {

    if (MPI_Init(&argc, &argv)){
		printf("Error MPI_Init\n");
		exit(-1);
	}
    if (MPI_Comm_size(MPI_COMM_WORLD, &process_count)){
		printf("Error MPI_Comm_Size\n");
		exit(-1);
	}
    if (MPI_Comm_rank(MPI_COMM_WORLD, &my_rank)){
		printf("Error MPI_Comm_rank\n");
		exit(-1);
	}

    if (argc < 4){
        printf("Usage: color2Grayscale ${input color image file} ${output grayscale image file} ${image type}\n Image Types:\n\t1: PGN\n\t2: JPG");
	exit(-1);
    }

    int width, height, channels, type;
    char  in_name[MAX_NAME_LENGTH], out_name[MAX_NAME_LENGTH];
    strcpy(in_name, argv[1]);
    strcpy(out_name, argv[2]);
    type = atoi(argv[3]);
    unsigned char *color_img;
    if(my_rank==0)
    {
        // load and conver the image to 3 channels (ignore the transparancy channel)
        color_img= stbi_load(in_name, &width, &height, &channels, 0); 
        
        if(color_img == NULL) {
            printf("Error in loading the image\n");
            exit(-1);
        }
        printf("Loaded image %s with a width of %dpx, a height of %dpx and %d channels\n", in_name, width, height, channels);
    }
    MPI_Bcast(&height,1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&width,1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels,1, MPI_INT, 0, MPI_COMM_WORLD);
    // Convert the input image to gray
    int gray_channels = channels == 4 ? 2 : 1;
    size_t gray_img_size = width * height * gray_channels;
    size_t color_img_size=gray_img_size* DESIRED_CHANNELS;

    size_t sub_gray_img_size=gray_img_size/process_count;
    size_t sub_color_img_size=color_img_size/process_count;

    int element_per_proc=width*height*DESIRED_CHANNELS/process_count;
   
    if(color_img_size%(element_per_proc/3)!=0 &&  my_rank==0)
    {
	printf("\nPixels are not equally distributed among processor, so image is not properly grayscaled\n\n");
    }	
    unsigned char *sub_color_img = (unsigned char *)malloc(sub_color_img_size);
    unsigned char *sub_gray_img=(unsigned char *)malloc(sub_gray_img_size);
    unsigned char *gray_img= (unsigned char *)malloc(gray_img_size);
   
    if(my_rank==0)
    {      
         
        if(gray_img == NULL) {
            printf("Unable to allocate memory for the gray image.\n");
            exit(1);
        }
        printf("Create a image array with a width of %dpx, a height of %dpx and %d channels\n", width, height, gray_channels);
    }
    
    MPI_Scatter(color_img, sub_color_img_size, MPI_CHAR, sub_color_img, sub_color_img_size, MPI_CHAR, 0, MPI_COMM_WORLD);
   
    colorToGrayscale(sub_gray_img, sub_color_img, element_per_proc);

    MPI_Gather(sub_gray_img,sub_gray_img_size , MPI_CHAR, gray_img, sub_gray_img_size, MPI_CHAR, 0, MPI_COMM_WORLD);
    if(my_rank==0)
    {
        if (type == IS_PNG)
            stbi_write_png(out_name, width, height, gray_channels, gray_img, width * gray_channels);
        else
            if (type == IS_JPG)
                stbi_write_jpg(out_name, width, height, gray_channels, gray_img, 100); //The last parameter of the stbi_write_jpg function is a quality parameter that goes from 1 to 100. Since JPG is a lossy image format, you can chose how much data is dropped at save time. Lower quality means smaller image size on disk and lower visual image quality.
        printf("Wrote image %s with a width of %dpx, a height of %dpx and %d channels\n", out_name, width, height, channels);
    }

    stbi_image_free(gray_img); 

    fflush(stdout);
	MPI_Finalize();
}

void colorToGrayscale(unsigned char *gray_img, unsigned char * color_img, int element_per_proc)
{
    unsigned char pixel[DESIRED_CHANNELS];
    
    for (int i = 0; i < element_per_proc; i=i+3)
    {
        pixel[0] = color_img[i];
        pixel[1] = color_img[i+1];
        pixel[2] = color_img[i+2];
        gray_img[i/3] = pixel[0] * 0.3 + pixel[1] * 0.58 + pixel[2] * 0.11;
    }
}
