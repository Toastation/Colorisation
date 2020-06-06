/**
 * @file WelshColorisation.cpp
 * @brief 
 *
 */

#include "WelshColorisation.hpp"

#include <string.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>

#define DEFAULT_NEIGHBORHOOD_SIZE 5
#define DEFAULT_SAMPLES 256
#define DEFAULT_SAMPLING JITTERED
#define DEFAULT_MEAN_WEIGHT 0.5

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define IN_RECT(x, y, rx, ry, rw, rh) (x >= rx &&  x < rx + rw && y >= ry &&  y < ry + rh)

typedef std::vector<Rect2d> vec_swatch;

/**
 * @brief log the given error message to the standard error output
 * 
 * @param message the message to log
 */
void log_error(const char * message) {
    fprintf(stderr, "*** Colorisation error: %s\n", message);
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
 * @brief Allocate a params structure with default values
 * 
 * @return params the structure newly allocated 
 */
params create_default_params() {
    params prm = (params) malloc(sizeof(struct params_s));
    exit_if(prm == NULL, "Error allocating parameters structure");
    prm->mean_weight = DEFAULT_MEAN_WEIGHT;
    prm->neighborhood_window_size = DEFAULT_NEIGHBORHOOD_SIZE;
    prm->samples = DEFAULT_SAMPLES;
    prm->sampling = DEFAULT_SAMPLING;
    prm->show_samples = false;
    prm->verbose = false;
    return prm;
}

/**
 * @brief Loads the image matrices with the given image path
 * 
 * @param src_path The path to the source image (colored)
 * @param target_path The path to the target image (grayscale)
 * @param src The source image matrix
 * @param target The target image matrix
 */
void load_matrices(const char * src_path, const char * target_path, Mat& src, Mat& target) {
    src = imread(samples::findFile(src_path));
    target = imread(samples::findFile(target_path));
    exit_if(src.empty(), "Cannot load source image");    
    exit_if(target.empty(), "Cannot load target image");
}

/**
 * @brief Get the sub matrices corresponding the swatches from the source and target image
 * given the swatches data from the colorisation structure
 * 
 * @param colorisation The parameters of the colorisation
 * @param target_swatch_mat The list of swatch matrices for the source 
 * @param target_swatch_mat The list of swatch matrices for the target
 */
void get_swatch_matrices(Mat& src, Mat& target, const vec_swatch& src_swatches, const vec_swatch& target_swatches, std::vector<Mat>& src_swatch_mat, std::vector<Mat>& target_swatch_mat) {
    for (std::size_t i = 0; i < src_swatches.size(); i++) {
        src_swatch_mat.push_back(src(src_swatches[i])); 
        target_swatch_mat.push_back(target(target_swatches[i])); 
    } 
}

/**
 * @brief Remap the luminance distribution of the source image to fit the one of the target image.
 * See Hertzmann et al. paper "Image analogies" (2001)
 * 
 * @param colorisation The parameters of the colorisation
 */
void luminance_remap(Mat& src, Mat& target) {
    Scalar src_mean, src_stddev, target_mean, target_stddev;
    meanStdDev(src, src_mean, src_stddev);
    meanStdDev(target, target_mean, target_stddev);
    int cols = src.cols;
    int rows = src.rows;
    double ratio = target_stddev[0] / src_stddev[0]; // ratio of the luminance std deviation
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            Vec3b color = src.at<Vec3b>(y,x);
            color[0] = ratio * (color[0] - src_mean[0]) + target_mean[0];
            src.at<Vec3b>(y, x) = color;
        }
    }
}

/**
 * @brief returns a rectangle representing the given pixel neighborhood 
 * 
 * @param img 
 * @param x 
 * @param y 
 * @param prm 
 * @return Rect 
 */
Rect get_neighborhood_rect(const Mat& img, int x, int y, params prm) {
    int half_size = prm->neighborhood_window_size / 2;
    int size = prm->neighborhood_window_size;
    int tx = CLAMP(x - half_size, 0, img.cols);
    int ty = CLAMP(y - half_size, 0, img.rows);
    int width = img.cols - (tx + size) < 0 ? img.cols - tx : prm->neighborhood_window_size; 
    int height = img.rows - (ty + size) < 0 ? img.rows - ty : prm->neighborhood_window_size; 
    return Rect(tx, ty, width, height);
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
Vec2d compute_pixel_neighborhood_stat(params prm, Mat& img, int x, int y) {
    Rect rect = get_neighborhood_rect(img, x, y, prm);
    Mat sub_mat = img(rect);
    Scalar mean, stddev;
    meanStdDev(sub_mat, mean, stddev);
    return Vec2d(mean[0], stddev[0]);
}

/**
 * @brief Compute the neighborhood stats of all pixels in the colored image.
 * 
 * @param colorisation 
 * @param neighborhood_stats 
 */
void brute_force_sampling(Mat& src, params prm, Vec2d * neighborhood_stats, std::vector<Vec2i>& neighborhood_pos) {
    for (uint i = 0; i < prm->samples; i++) {
        int x = i % src.rows;
        int y = i / src.rows;
        neighborhood_stats[i] = compute_pixel_neighborhood_stat(prm, src, x, y);
        neighborhood_pos.push_back(Vec2i(x, y));
    }
}

/**
 * @brief Compute the neighborhood stats of pixels in the colored image based on random
 * jittered sampling.
 * 
 * @param colorisation 
 * @param neighborhood_stats 
 */
void jittered_sampling(Mat& src, params prm, Vec2d * neighborhood_stats, std::vector<Vec2i>& neighborhood_pos) {
    int n = sqrt(prm->samples);
    int grid_x = src.cols / n; 
    int grid_y = src.rows / n;
    int index = 0;
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            int pos_x = x * grid_x + (rand() % grid_x);
            int pos_y = y * grid_y + (rand() % grid_y);
            if (neighborhood_stats != NULL)
                neighborhood_stats[index] = compute_pixel_neighborhood_stat(prm, src, pos_x, pos_y);
            neighborhood_pos.push_back(Vec2i(pos_x, pos_y));
            index++;
        }
    } 
}

/**
 * @brief With the given pixel stats, find its closest match in the colored image.
 * The closest match is based on the pixel's neighborhood weighted mean and standard
 * deviation (by default, 50% for both).
 * 
 * @param colorisation 
 * @param target_stats 
 * @return int 
 */
int find_best_matching_pixel(params prm, Vec2d * neighborhood_stat, int size, const Vec2d& target_stats) {
    double diffs[size];
    double target_mean, src_mean, target_stddev, src_stddev;
    double weight_mean = prm->mean_weight;
    double weight_dev = 1.0 - prm->mean_weight; 
    
    // compute the weighted square difference between the samples and given neighborhoods
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        target_mean = target_stats[0];
        target_stddev = target_stats[1];
        src_mean = neighborhood_stat[i][0];
        src_stddev = neighborhood_stat[i][1];
        diffs[i] = (weight_mean * (target_mean - src_mean) * (target_mean - src_mean)) 
                + (weight_dev * (target_stddev - src_stddev) * (target_stddev - src_stddev)); 
    }

    // find the minimum value among the squared differences
    double min_diff = INT_MAX;
    int min_index = 0;
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        if (diffs[i] < min_diff) {
            min_diff = diffs[i];
            min_index = i;
        }
    }
    return min_index;
}

/**
 * @brief Compute the neighborhood stats for each pixel in the grayscale image
 * and find its best matching pixel in the colored image.
 * The chromaticity is then transferred from the best match to the grayscale pixel (A and B channels).  
 * 
 * @param colorisation 
 * @param neighborhood_stat 
 */
void transfer_color(Mat& src, Mat& target, params prm, Vec2d * neighborhood_stat, const std::vector<Vec2i>& neighborhood_pos) {
    // #pragma omp parallel for collapse(2)
    for (int x = 0; x < target.cols; x++) {
        for (int y = 0; y < target.rows; y++) {
            Vec2d stats = compute_pixel_neighborhood_stat(prm, target, x, y);
            int match_index = find_best_matching_pixel(prm, neighborhood_stat, prm->samples, stats);
            if (prm->verbose) printf("Matching position index: %d (target: %d %d)\n", match_index, x, y);
            Vec3b color = target.at<Vec3b>(y, x);
            Vec3b matching_color = src.at<Vec3b>(neighborhood_pos[match_index][1], neighborhood_pos[match_index][0]);
            color[1] = matching_color[1];
            color[2] = matching_color[2];
            target.at<Vec3b>(y, x) = color; 
        }
    }
}

void sample_and_transfer(Mat& src, Mat& target, params prm) {
    // match source and target luminance histogram
    luminance_remap(src, target);
    
    // sample pixels in source image and compute their neighborhood stats
    Vec2d * neighborhood_stats = (Vec2d *) malloc(sizeof(Vec2d) * prm->samples);
    exit_if(neighborhood_stats == NULL, "Error allocating neighborhood stat array\n");
    std::vector<Vec2i> neighborhood_pos;
    switch (prm->sampling) {
        case JITTERED:
            jittered_sampling(src, prm, neighborhood_stats, neighborhood_pos);
            break;
        case BRUTE_FORCE:
            brute_force_sampling(src, prm, neighborhood_stats, neighborhood_pos);
            break;
    }

    // find the best color match for each grayscale pixel and transfer its color
    transfer_color(src, target, prm, neighborhood_stats, neighborhood_pos);
    free(neighborhood_stats);
}

/**
 * @brief 
 * 
 * @param target 
 * @param x 
 * @param y 
 * @param sx 
 * @param sy 
 * @param prm 
 * @return Vec4i 
 */
Vec4i get_min_neighbordhood_rect(const Mat& target, int x, int y, int sx, int sy, params prm) {
    int half_size = prm->neighborhood_window_size / 2;
    int ox = std::min(std::min(x, sx), half_size);
    int oy = std::min(std::min(y, sy), half_size);
    int sizex = std::min((int)prm->neighborhood_window_size, std::min(target.cols - (x - ox), target.cols - (sx - ox)));
    int sizey = std::min((int)prm->neighborhood_window_size, std::min(target.rows - (y - oy), target.rows - (sy - oy)));
    return Vec4i(ox, oy, sizex, sizey);
}

double compute_sq_diff(const Mat& target, const Vec4i& rect, int x, int y, int sx, int sy) {
    double sum = 0.0;
    Vec3b colored, gray;
    #pragma omp parallel for collapse(2)
    for (int xx = 0; xx < rect[2]; xx++) {
        for (int yy = 0; yy < rect[3]; yy++) {
            gray = target.at<Vec3b>(x+xx-rect[0], y+yy-rect[1]);
            colored = target.at<Vec3b>(sx+xx-rect[0], sy+yy-rect[1]);
            sum += (gray[0] - colored[0]) * (gray[0] - colored[0]);
        }
    }
    return sum;
}

/**
 * @brief Get the coordinate of the colorised pixel (in a swatch) that minimise the error distance.
 * See Welsh et al. paper for the definition of the error distance.
 * 
 * @param pixel grayscale pixel to compute the error distance from
 * @param target_swatches list of all swatches in the target image
 * @return the coordinate of the colorised pixel (in a swatch) that minimise the error distance and the index of the swatch the pixel is from
 */
int get_minimum_error_distance(const Mat& target, int x, int y, const std::vector<Vec2i>& swatch_samples, const vec_swatch& target_rect, params prm) {
    double min_error_dist = 0xFFFF; // minimum error distance between the pixel neighborhood and a colorised pixel neighborhood
    double min_index = 0;
    // iterate over all colorised pixel and compute the error distance between the given pixel and the iterated pixel
    Vec2i sample;
    Vec4i rect;
    // #pragma omp parallel for
    for (size_t j = 0; j < swatch_samples.size(); j++) {
        sample = swatch_samples[j];
        rect = get_min_neighbordhood_rect(target, x, y, sample[0], sample[1], prm);
        Mat a = target(Rect(x-rect[0], y-rect[1], rect[2], rect[3]));
        Mat b = target(Rect(sample[0]-rect[0], sample[1]-rect[1], rect[2], rect[3]));
        Mat c;
        cv::absdiff(a, b, c);
        c.convertTo(c, CV_32F);  // cannot make a square on 8 bits
        c = c.mul(c);   
        Scalar s = sum(c);
        // sq_sum = compute_sq_diff(target, rect, x, y, sample[0], sample[1]);
        if (s[0] < min_error_dist) {
            min_error_dist = s[0];
            min_index = j;
        }
    }
    return min_index;
}

void diffuse_color(Mat& target, std::vector<Mat>& target_swatches, const vec_swatch& target_rect, params prm) {
    // get all samples from all swatches
    std::vector<Vec2i> swatch_samples; 
    for (size_t i = 0; i < target_swatches.size(); i++) {
        std::vector<Vec2i> neighborhood_pos;
        jittered_sampling(target_swatches[i], prm, NULL, neighborhood_pos);
        for (size_t j = 0; j < neighborhood_pos.size(); j++) {
            neighborhood_pos[j][0] += target_rect[i].x;
            neighborhood_pos[j][1] += target_rect[i].y;
        }
        swatch_samples.insert(swatch_samples.end(), neighborhood_pos.begin(), neighborhood_pos.end());
    }

    // compute their stats
    Vec2d * stats = (Vec2d *) malloc(sizeof(Vec2d) * swatch_samples.size());
    exit_if(stats == NULL, "error allocating stats array");
    for (size_t i = 0; i < swatch_samples.size(); i++) {
        stats[i] = compute_pixel_neighborhood_stat(prm, target, swatch_samples[i][0], swatch_samples[i][1]);
    }

    // #pragma omp parallel for collapse(2)
    for (int x = 0; x < target.cols; x++) {
        for (int y = 0; y < target.rows; y++) {
            Vec3b pixel = target.at<Vec3b>(y, x);
            if (pixel[1] != pixel[2] || pixel[1] != 128) continue; // skip already colorised pixels
            // Vec2d pixel_stats = compute_pixel_neighborhood_stat(prm, target, x, y);
            // int match_index = find_best_matching_pixel(prm, stats, swatch_samples.size(), pixel_stats);
            int match_index = get_minimum_error_distance(target, x, y, swatch_samples, target_rect, prm);
            if (prm->verbose) printf("Matched index %d (target: %d %d)", match_index, x, y);
            Vec3b matching_color = target.at<Vec3b>(swatch_samples[match_index][1], swatch_samples[match_index][0]);
            pixel[1] = matching_color[1];
            pixel[2] = matching_color[2];
            target.at<Vec3b>(y, x) = pixel; 
        }
    }
    free(stats);
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
void run(Mat& src, Mat& target, params prm) {
    // convert source and target images to LAB col or space
    cvtColor(src, src, COLOR_BGR2Lab);
    cvtColor(target, target, COLOR_BGR2Lab);    

    sample_and_transfer(src, target, prm);

    // LAB->BGR conversion
    cvtColor(target, target, COLOR_Lab2BGR);

    if (prm->show_samples) {
        cvtColor(src, src, COLOR_Lab2BGR);
        imwrite("samples.png", src);
    }    
}

/**
 * @brief Run the welsh algorithm with swatches
 * 
 * @param colorisation 
 */
void run_swatch(Mat& src, Mat& target, const vec_swatch& src_swatches, const vec_swatch& target_swatches, params prm) {
    // convert source and target images to LAB color space
    cvtColor(src, src, COLOR_BGR2Lab);
    cvtColor(target, target, COLOR_BGR2Lab);   
    
    // compute the sub matrices for each swatch
    std::vector<Mat> src_swatch_mat, target_swatch_mat;
    get_swatch_matrices(src, target, src_swatches, target_swatches, src_swatch_mat, target_swatch_mat);
    
    // apply general algo on each swatch (luminance remap + sampling + color transfer)
    int nb_swatches = src_swatches.size();
    int samples_save = prm->samples;
    prm->samples = 64; // temporarily lower samples for the swatch color transfer
    for (int i = 0; i < nb_swatches; i++) {
        sample_and_transfer(src_swatch_mat[i], target_swatch_mat[i], prm);
    }

    diffuse_color(target, target_swatch_mat, target_swatches, prm);
    prm->samples = samples_save;

    // LAB->BGR conversion
    cvtColor(target, target, COLOR_Lab2BGR);    
}

///////////////////
// API FUNCTIONS //
///////////////////

void welsh_colorisation(Mat& source_img, Mat& target_img, const char * dst_img, params prm) {
    if (prm == NULL) prm = create_default_params();
    run(source_img, target_img, prm);
    imwrite(dst_img, target_img);
}

void welsh_colorisation_swatches(Mat& source_img, Mat& target_img, const char * dst_img, params prm, const vec_swatch& src_rect, const vec_swatch& target_rect) {
    exit_if(src_rect.size() != target_rect.size(), "Error: the number of swatches in the source and target does not match.");
    if (prm == NULL) prm = create_default_params(); 
    std::cout << src_rect.size() << std::endl;
    run_swatch(source_img, target_img, src_rect, target_rect, prm);
    imwrite(dst_img, target_img);
}