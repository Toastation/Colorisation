#include "WelshColoration.hpp"

struct colorisation {
    unsigned int neighborhood_window_size;  // size of the neighborhood window for pixel matching
    Mat * src;                              // the source image (colored image)
    Mat * target;                           // the target image (greyscale image)
    Mat * dst;                              // the result image (colored version of the target image)
};

void log_error(const char * message) {
    fprintf(stderr, "*** Colorisation error: %s", message);
}

void exit_if(bool cond, const char * message) {
    if (cond) {
        log_error(message);
        exit(EXIT_FAILURE);
    }
}

colorisation_s create_colorisation_struct(Mat * src, Mat * target, Mat * dst, unsigned int neighborhood_window_size) {
    colorisation_s colorisation = (colorisation_s) malloc(sizeof(struct colorisation));
    exit_if(!colorisation, "Cannot allocate colorisation structure");
    colorisation->neighborhood_window_size = 5;
    colorisation->src = src;
    colorisation->target = target;
    colorisation->dst = dst;
    return colorisation;
}


/** 
 * API FUNCTIONS
 */

void welsh_coloration(const char * source_img, const char * target_img) {
    // load images and initialise parameters
    Mat src, target, dst;
    src = imread(samples::findFile(source_img));
    target = imread(samples::findFile(source_img));
    exit_if(src.empty(), "Cannot load source image");    
    exit_if(target.empty(), "Cannot load target image");    
    colorisation_s colorisation = create_colorisation_struct(&src, &target, &dst, 5);

    // convert source and target to lab color space
    cvtColor(*(colorisation->src), *(colorisation->src), COLOR_BGR2Lab);
    cvtColor(*(colorisation->target), *(colorisation->target), COLOR_BGR2Lab);    
    dst = Mat(target);

    free(colorisation);
}
