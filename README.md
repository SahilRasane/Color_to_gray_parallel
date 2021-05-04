# Data Parallelism in the Color to Greyscale Image Conversion
The aim of the project is to -
1. Write a CUDA code  to parallelize the color-to-greyscale conversion by remotely running in our course devices.

2. Write a MPI distributed-memory version, where the master process read the image file, scatters the image pixels to slave processes, gather the converted pixel data after each process finishes the calculation.

3. Write an OpenMP shared-memory version. Using diverse OpenMP directives, for example, parallel for.

Refer to the readme in the folder for execution
