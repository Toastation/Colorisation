#ifndef WELSH_COLORISATION_HPP
#define WELSH_COLORISATION_HPP

#include <vector>

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#define DEFAULT_NEIGHBORHOOD_SIZE 5
#define DEFAULT_SAMPLES 225
#define DEFAULT_SAMPLING JITTERED
#define DEFAULT_MEAN_WEIGHT 0.5

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
};

typedef struct params_s * params;

/**
 * @brief General version of the Welsh et al. colorisation algorithm (no swatches)
 * 
 * @param source_img The path of the colored image 
 * @param target_img The path of the grayscale image
 * @param dst_img The path of the result image
 * @param neighborhood_size The size of the pixel neighborhood window
 * @param samples The number of samples taken from the colored images to find the best luminance match 
 */
void welsh_colorisation(const char * source_img, const char * target_img, const char * dst_img, params prm);

void welsh_colorisation_swatches(const char * source_img, const char * target_img, const char * dst_img, params prm, const std::vector<int>& src_rect, const std::vector<int>& target_rect);

#endif