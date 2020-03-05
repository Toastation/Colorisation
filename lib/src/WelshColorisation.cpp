/**
 * @file WelshColorisation.cpp
 * @brief 
 *
 * TODO :
 * - parameterize weights in find_best_match
 * - test cases where images aren't of the same size
 * - optimize
 * 
 */

#include "WelshColorisation.hpp"

#include <string.h>
#include <iostream>
#include <algorithm>

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/**
 * @brief how to sample pixel for the matching process 
 * 
 */
enum sampling_method {
    JITTERED,       // random jittered sampling
    BRUTE_FORCE     // sample all pixels
};

/**
 * @brief Contains the images and parameters of the colorisation
 * 
 */
struct colorisation {
    uint neighborhood_window_size;          // size of the neighborhood window for pixel matching
    uint samples;                           // number of samples for the pixel matching (must be a square number)
    sampling_method sampling;               // how to sample pixel for the matching process
    Mat * src;                              // the source image (colored image)
    Mat * target;                           // the target image (greyscale image)
};

/**
 * @brief log the given error message to the standard error output
 * 
 * @param message the message to log
 */
void log_error(const char * message) {
    fprintf(stderr, "*** Colorisation error: %s", message);
}

/**
 * @brief exit if the given condition is true, and log the given error message
 * 
 * @param cond the stopping condition
 * @param message the error message log if the condition is true 
 */
void exit_if(bool cond, const char * message) {
    if (cond) {
        log_error(message);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Create a colorisation struct object, see the colorisation struct brief
 * 
 * @param src 
 * @param target 
 * @param dst 
 * @param neighborhood_window_size 
 * @return colorisation_s 
 */
colorisation_s create_colorisation_struct(Mat * src, Mat * target, uint neighborhood_window_size, uint samples, sampling_method sampling) {
    colorisation_s colorisation = (colorisation_s) malloc(sizeof(struct colorisation));
    exit_if(!colorisation, "Cannot allocate colorisation structure");
    colorisation->neighborhood_window_size = 5;
    colorisation->samples = (sampling == BRUTE_FORCE) ? (src->cols * src->rows) : samples;
    colorisation->sampling = sampling;
    colorisation->src = src;
    colorisation->target = target;
    return colorisation;
}

/**
 * @brief Remap the luminance distribution of the source image to fit the one of the target image.
 * See Hertzmann et al. paper "Image analogies" (2001)
 * 
 * @param colorisation The parameters of the colorisation
 */
void luminance_remap(colorisation_s colorisation) {
    Scalar src_mean, src_stddev, target_mean, target_stddev;
    meanStdDev(*(colorisation->src), src_mean, src_stddev);
    meanStdDev(*(colorisation->target), target_mean, target_stddev);
    int cols = colorisation->src->cols;
    int rows = colorisation->src->rows;
    double ratio = target_stddev[0] / src_stddev[0]; // ratio of the luminance std deviation
    // std::cout << src_stddev << std::endl;
    // std::cout << src_mean << std::endl;    
    // std::cout << target_stddev << std::endl;    
    // std::cout << target_mean << std::endl;    
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            Vec3b color = colorisation->src->at<Vec3b>(y,x);
            color[0] = ratio * (color[0] - src_mean[0]) + target_mean[0];
            colorisation->src->at<Vec3b>(y, x) = color;
        }
    }
    // meanStdDev(*(colorisation->src), src_mean, src_stddev);
    // std::cout << src_stddev << std::endl;    
    // std::cout << src_mean << std::endl;    
}

/**
 * @brief Computes the mean luminance and standard deviation of the neighborhood of the given pixel.
 * The neighborhood is a square centered on the given pixel and its size is stored in the colorisation struct.
 * 
 * @param colorisation 
 * @param x The x coord of the pixel
 * @param y The y coord of the pixel
 * @return Vec2d A vector representing the (mean, stddev) luminance of the pixel's neighborhood
 */
Vec2d compute_pixel_neighborhood_stat(colorisation_s colorisation, Mat * img, int x, int y) {
    int half_size = colorisation->neighborhood_window_size / 2;
    int size = colorisation->neighborhood_window_size;
    int tx = CLAMP(x - half_size, 0, img->cols);
    int ty = CLAMP(y - half_size, 0, img->rows);
    int width = img->cols - (tx + size) < 0 ? img->cols - tx : colorisation->neighborhood_window_size; 
    int height = img->rows - (ty + size) < 0 ? img->rows - ty : colorisation->neighborhood_window_size; 
    Rect rect(tx, ty, width, height);
    Mat sub_mat = (*(img))(rect);
    Scalar mean, stddev;
    meanStdDev(sub_mat, mean, stddev);
    return Vec2d(mean[0], stddev[0]);
}

/**
 * @brief 
 * 
 * @param colorisation 
 * @param neighborhood_stats 
 */
void brute_force_sampling(colorisation_s colorisation, Vec2d * neighborhood_stats, Vec2i * neighborhood_pos) {
    for (uint i = 0; i < colorisation->samples; i++) {
        int x = i % colorisation->src->rows;
        int y = i / colorisation->src->rows;
        neighborhood_stats[i] = compute_pixel_neighborhood_stat(colorisation, colorisation->src, x, y);
        neighborhood_pos[i] = Vec2i(x, y);
    }
}

/**
 * @brief 
 * 
 * @param colorisation 
 * @param neighborhood_stats 
 */
void jittered_sampling(colorisation_s colorisation, Vec2d * neighborhood_stats, Vec2i * neighborhood_pos) {
    int n = sqrt(colorisation->samples);
    int grid_x = colorisation->src->cols / n; 
    int grid_y = colorisation->src->rows / n;
    int index = 0;
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            int pos_x = x * grid_x + (rand() % grid_x);
            int pos_y = y * grid_y + (rand() % grid_y);
            neighborhood_stats[index] = compute_pixel_neighborhood_stat(colorisation, colorisation->src, pos_x, pos_y);
            neighborhood_pos[index] = Vec2i(pos_x, pos_y);
            index++;
        }
    } 
}

/**
 * @brief 
 * TODO: parameterize weights
 * 
 * @param colorisation 
 * @param target_stats 
 * @return int 
 */
int find_best_matching_pixel(colorisation_s colorisation, Vec2d * neighborhood_stat, Vec2d target_stats) {
    double diffs[colorisation->samples];
    for (uint i = 0; i < colorisation->samples; i++) {
        diffs[i] = (0.5 * (target_stats[0] - neighborhood_stat[i][0]) * (target_stats[0] - neighborhood_stat[i][0])) 
            + (0.5 * (target_stats[1] - neighborhood_stat[i][1]) * (target_stats[1] - neighborhood_stat[i][1])); 
    }
    double min_diff = INT_MAX;
    int min_index = 0;
    for (uint i = 0; i < colorisation->samples; i++) {
        if (diffs[i] < min_diff) {
            min_diff = diffs[i];
            min_index = i;
        }
    }
    return min_index;
}

/**
 * @brief 
 * 
 * @param colorisation 
 * @param neighborhood_stat 
 */
void transfer_color(colorisation_s colorisation, Vec2d * neighborhood_stat, Vec2i * neighborhood_pos) {
    for (int x = 0; x < colorisation->target->cols; x++) {
        for (int y = 0; y < colorisation->target->rows; y++) {
            Vec2d stats = compute_pixel_neighborhood_stat(colorisation, colorisation->target, x, y);
            int match_index = find_best_matching_pixel(colorisation, neighborhood_stat, stats);
            Vec3b color = colorisation->target->at<Vec3b>(y, x);
            Vec3b matching_color = colorisation->src->at<Vec3b>(neighborhood_pos[match_index][1], neighborhood_pos[match_index][0]);
            color[1] = matching_color[1];
            color[2] = matching_color[2];
            colorisation->target->at<Vec3b>(y, x) = color; 
        }
    }
}

/**
 * @brief Run the welsh colorisation algorithm
 * 1. Convert the source (color) and target (grayscale) images to LAB color space
 * 2. Match the source luminance histogram to the grayscale target 
 * 3. Compute the neighborhood stats (mean, stddev) of pixels in the source image
 * 4. For each pixel in the target, find a match in the source based on the result of step 3.
 * 5. Transfer the colors (A,B channels) of the matching pixel to the target.
 * 
    cvtColor(*(colorisation->src), *(colorisation->src), COLOR_BGR2Lab);
 * @param colorisation 
 */
void run(colorisation_s colorisation) {
    // convert source and target images to LAB color space
    cvtColor(*(colorisation->src), *(colorisation->src), COLOR_BGR2Lab);
    cvtColor(*(colorisation->target), *(colorisation->target), COLOR_BGR2Lab);    

    // match source and target luminance histogram
    luminance_remap(colorisation);
    
    // sample pixels in source image and compute their neighborhood stats
    Vec2d * neighborhood_stats = (Vec2d *) malloc(sizeof(Vec2d) * colorisation->samples);
    Vec2i * neighborhood_pos = (Vec2i *) malloc(sizeof(Vec2i) * colorisation->samples);
    exit_if(neighborhood_stats == NULL, "Error allocating neighborhood stat array\n");
    exit_if(neighborhood_pos == NULL, "Error allocating neighborhood pos array\n");
    switch (colorisation->sampling) {
        case JITTERED:
            jittered_sampling(colorisation, neighborhood_stats, neighborhood_pos);
            break;
        case BRUTE_FORCE:
            brute_force_sampling(colorisation, neighborhood_stats, neighborhood_pos);
            break;
    }

    imwrite("1.jpg", *(colorisation->target));

    transfer_color(colorisation, neighborhood_stats, neighborhood_pos);

    imwrite("2.jpg", *(colorisation->target));

    cvtColor(*(colorisation->target), *(colorisation->target), COLOR_Lab2BGR);    
}

///////////////////
// API FUNCTIONS //
///////////////////

void welsh_colorisation(const char * source_img, const char * target_img, const char * dst_img) {
    // load images and initialise parameters
    Mat src, target;
    src = imread(samples::findFile(source_img));
    target = imread(samples::findFile(target_img));
    exit_if(src.empty(), "Cannot load source image");    
    exit_if(target.empty(), "Cannot load target image");    
    colorisation_s colorisation = create_colorisation_struct(&src, &target, 5, 529, JITTERED);

    run(colorisation);

    free(colorisation);
    imwrite(dst_img, target);
}
