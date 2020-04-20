#ifndef WELSH_COLORISATION_HPP
#define WELSH_COLORISATION_HPP

#include <vector>

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

using namespace cv;

/**
 * @brief how to sample pixel for the matching process 
 * 
 */
enum sampling_method {
    JITTERED,       // random jittered sampling
    BRUTE_FORCE     // sample all pixels
};

/**
 * @brief parameters of the colorisation algorithm
 * 
 */
struct params_s {
    uint neighborhood_window_size;  // size of the neighborhood window for pixel matching
    uint samples;                   // number of samples for the pixel matching (must be a square number)
    sampling_method sampling;       // how to sample pixel for the matching process
    double mean_weight;             // weight of the mean luminance for determining the best match (stddev weight = 1-mean_weight)    
    bool show_samples;              // if true, samples points are colored
    bool verbose;                   // if true, print information about the color transfer
};

typedef struct params_s * params;

/**
 * @brief General version of the Welsh et al. colorisation algorithm (no swatches)
 * 
 * @param source_img The mat of the colored image 
 * @param target_img The mat of the grayscale image
 * @param dst_img The path of the result image
 * @param neighborhood_size The size of the pixel neighborhood window
 * @param samples The number of samples taken from the colored images to find the best luminance match 
 */
void welsh_colorisation(Mat& source_img, Mat& target_img, const char * dst_img, params prm);

/**
 * @brief 
 * 
 * @param source_img 
 * @param target_img 
 * @param dst_img 
 * @param prm 
 * @param src_rect 
 * @param target_rect 
 */
void welsh_colorisation_swatches(Mat& source_img, Mat& target_img, const char * dst_img, params prm, const std::vector<Rect2d>& src_rect, const std::vector<Rect2d>& target_rect);

/**
 * @brief Create a default params structure
 * 
 * @return params 
 */
params create_default_params();

#endif