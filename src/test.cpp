#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "WelshColorisation.hpp"

using namespace cv;

int main(int argc, char ** argv) {
    const char window_name[] = "Welsh Colorisation"; 
    namedWindow(window_name, WINDOW_AUTOSIZE);

    int opt; 
    const char * color_path = NULL;
    const char * gray_path = NULL;
    const char * dest_path = NULL;
    int swatches = 0;
    params prm = create_default_params();

    // parse params
    while((opt = getopt(argc, argv, ":c:g:d:w:svr:")) != -1)  
    {  
        switch(opt)  
        {    
            case 'c':  
                color_path = optarg;
                break;  
            case 'g':  
                gray_path = optarg;
                break;  
            case 'd':
                dest_path = optarg;
                break;
            case 'w':
                prm->neighborhood_window_size = atoi(optarg);
                break;
            case 's':
                prm->show_samples = true;
                break;
            case 'v':
                prm->verbose = true;
                break;
            case 'r':
                swatches = atoi(optarg);
            case ':':  
                printf("value required\n");  
                break;  
            case '?':  
                printf("unknown option: %c\n", optopt); 
                break;  
        }  
    }  

    if (color_path == NULL || gray_path == NULL) {
        fprintf(stderr, "-c and -g option are necessary\n");
        exit(1);
    }

    if (dest_path == NULL) {
        dest_path = "./a.png";
    }

    // ask user for swatches
    Mat src = imread(color_path);
    Mat target = imread(gray_path);
    std::vector<Rect2d> src_swatches, target_swatches;
    for (int i = 0; i < swatches; i++) {
        Rect2d r1 = selectROI(src, true, false);
        Rect2d r2 = selectROI(target, true, false);
        src_swatches.push_back(r1);
        target_swatches.push_back(r2);
    }

    if (swatches == 0)
        welsh_colorisation(src, target, dest_path, prm);
    else if (swatches > 0)
        welsh_colorisation_swatches(src, target, dest_path, prm, src_swatches, target_swatches);

    Mat result = imread(dest_path);
    std::cout << "done!" << std::endl;
    // imshow(window_name, result);
    // waitKey(0);
}
