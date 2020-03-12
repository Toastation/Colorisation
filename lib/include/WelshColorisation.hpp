#ifndef WELSH_COLORISATION_HPP
#define WELSH_COLORISATION_HPP

#include <stdlib.h>
#include <stdio.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

using namespace cv;

typedef struct colorisation * colorisation_s;

/**
 * @brief 
 * 
 * @param source_img The path of the colored image 
 * @param target_img The path of the grayscale image
 * @param dst_img The path of the result image
 * @param neighborhood_size The size of the pixel neighborhood window
 * @param samples The number of samples taken from the colored images to find the best luminance match 
 */
void welsh_colorisation(const char * source_img, const char * target_img, const char * dst_img, int neighborhood_size, int samples);

#endif