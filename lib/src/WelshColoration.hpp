#ifndef WELSH_COLORATION_HPP
#define WELSH_COLORATION_HPP

#include <stdlib.h>
#include <stdio.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

using namespace cv;

typedef struct colorisation * colorisation_s;

/**
 * @brief 
 * 
 * @param source_img The color image 
 * @param target_img The grayscale image
 */
void welsh_coloration(const char * source_img, const char * target_img);

#endif