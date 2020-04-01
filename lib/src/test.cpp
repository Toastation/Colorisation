#include <iostream>

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "WelshColorisation.hpp"

using namespace cv;

int main(int argc, char ** argv) {
    const char window_name[] = "Welsh Colorisation"; 
    namedWindow(window_name, WINDOW_AUTOSIZE);
    if (argc != 4) {
        fprintf(stderr, "Usage: %s source_path target_path dst_path [x, y, w, h, x\', y\', w\', h\', ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char * src_path = argv[1];
    const char * target_path = argv[2]; 
    const char * dst_path = argv[3]; 
    welsh_colorisation(src_path, target_path, dst_path, NULL);
    // int src_rect_data[] = {168, 89, 100, 60, 229, 288, 43, 59, 87, 384, 49, 86};
    // std::vector<int> src_rect(src_rect_data, src_rect_data + sizeof(src_rect_data) / sizeof(src_rect_data[0]));
    // int target_rect_data[] = {168, 89, 100, 60, 229, 288, 43, 59, 87, 384, 49, 86};
    // std::vector<int> target_rect(target_rect_data, target_rect_data + sizeof(target_rect_data) / sizeof(target_rect_data[0]));
    // welsh_colorisation_swatches(src_path, target_path, dst_path, NULL, src_rect, target_rect);
    Mat result = imread(dst_path);
    imshow(window_name, result);
}

// using namespace std;
// using namespace cv;

// int DELAY_CAPTION = 1500;
// int DELAY_BLUR = 100;
// int MAX_KERNEL_LENGTH = 31;
// Mat src; Mat dst;
// char window_name[] = "Smoothing Demo";

// int display_caption( const char* caption );
// int display_dst( int delay );

// int main( int argc, char ** argv )
// {
//     namedWindow( window_name, WINDOW_AUTOSIZE );
//     const char* filename = argc >=2 ? argv[1] : "lena.jpg";
//     src = imread( samples::findFile( filename ), IMREAD_COLOR );
//     if (src.empty())
//     {
//         printf(" Error opening image\n");
//         printf(" Usage:\n %s [image_name-- default lena.jpg] \n", argv[0]);
//         return EXIT_FAILURE;
//     }
//     if( display_caption( "Original Image" ) != 0 )
//     {
//         return 0;
//     }
//     dst = src.clone();
//     if( display_dst( DELAY_CAPTION ) != 0 )
//     {
//         return 0;
//     }
//     if( display_caption( "Homogeneous Blur" ) != 0 )
//     {
//         return 0;
//     }
//     for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
//     {
//         blur( src, dst, Size( i, i ), Point(-1,-1) );
//         if( display_dst( DELAY_BLUR ) != 0 )
//         {
//             return 0;
//         }
//     }
//     if( display_caption( "Gaussian Blur" ) != 0 )
//     {
//         return 0;
//     }
//     for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
//     {
//         GaussianBlur( src, dst, Size( i, i ), 0, 0 );
//         if( display_dst( DELAY_BLUR ) != 0 )
//         {
//             return 0;
//         }
//     }
//     if( display_caption( "Median Blur" ) != 0 )
//     {
//         return 0;
//     }
//     for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
//     {
//         medianBlur ( src, dst, i );
//         if( display_dst( DELAY_BLUR ) != 0 )
//         {
//             return 0;
//         }
//     }
//     if( display_caption( "Bilateral Blur" ) != 0 )
//     {
//         return 0;
//     }
//     for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
//     {
//         bilateralFilter ( src, dst, i, i*2, i/2 );
//         if( display_dst( DELAY_BLUR ) != 0 )
//         {
//             return 0;
//         }
//     }
//     display_caption( "Done!" );
//     return 0;
// }

// int display_caption( const char* caption )
// {
//     dst = Mat::zeros( src.size(), src.type() );
//     putText( dst, caption,
//              Point( src.cols/4, src.rows/2),
//              FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255) );
//     return display_dst(DELAY_CAPTION);
// }
// int display_dst( int delay )
// {
//     imshow( window_name, dst );
//     int c = waitKey ( delay );
//     if( c >= 0 ) { return -1; }
//     return 0;
// }