# Transferring color to greyscale images
C++ implementation of Welsh et al. "Transferring color to greyscale images".


[Report](https://github.com/Toastation/Colorisation/blob/master/report_fr.pdf)

## Dependencies
* OpenCV 4.2
* OpenMP

## Running the application

```
mkdir build
cd build
cmake ..
make
./WelshColorisation -c path_coloured_image -g path_grayscale_image -d path_result_image [-r nb_swatches -w window_size]  
```
|Option|Description|Required|
|------|------|------| 
|-c ...|Path to the coloured image|Required|
|-g ...|Path to the grayscale image|Required|
|-d ...|Path of the result image|Optional|
|-r ...|Number of swatches|Optional|
|-w ...|Window size (odd integer)|Optional|
|-v|Verbose mode|Optional|

## Datasets
* https://geology.com/satellite/cities/ 
* http://www.vision.caltech.edu/Image_Datasets/Caltech101/
