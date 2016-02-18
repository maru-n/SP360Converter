#include "converter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

void convert_frame(Mat src_frame, Mat dst_frame, double angle_start, double angle_end, double radius_in, double radius_out, int n_split)
{
    double R = src_frame.cols / 2.;
    int w = double(dst_frame.cols);
    int h = double(dst_frame.rows);
    int pan_w = w * n_split;
    int pan_h = h / n_split;

    int split_row;
    double pan_i, pan_j, r, th, src_i, src_j;
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            split_row = j * n_split / h;
            pan_i = i + w * split_row;
            pan_j = j - h * split_row / n_split;
            r = R * (radius_in + (radius_out - radius_in) * pan_j / pan_h);
            th = angle_start + (angle_end - angle_start) * pan_i / pan_w;
            src_i = R + r * cos(th);
            src_j = R - r * sin(th);
            dst_frame.at<Vec3b>(Point(i, j)) = src_frame.at<Vec3b>(Point(src_i,src_j));
        }
    }
}


int convert(std::string src_file, std::string dst_file,
             int dst_width, int dst_height,
             int start_time, int end_time,
             double angle_start, double angle_end,
             double radius_in, double radius_out,
             int n_split,
             std::function<void(float)> callback)
{
    using namespace cv;
    VideoCapture cap(src_file.c_str());

    double fps = cap.get(CAP_PROP_FPS);
    int fourcc = VideoWriter::fourcc('a','v','c','1');
    VideoWriter writer(dst_file.c_str(), fourcc, fps, Size(dst_width, dst_height));

    cap.set(CAP_PROP_POS_MSEC, start_time);
    Mat frame;
    while (1) {
        double current_pos = cap.get(CV_CAP_PROP_POS_MSEC);
        if (current_pos > end_time) {
            break;
        }
        cap >> frame;
        if( frame.empty() )
            break;
        Mat new_frame = Mat(dst_height, dst_width, frame.type());
        convert_frame(frame, new_frame, angle_start, angle_end, radius_in, radius_out, n_split);
        writer << new_frame;
        float progress = (current_pos - start_time) / (end_time - start_time);
        callback(progress);
    }
    return 0;
}


/*
int main(int argc, char const *argv[]) {
    std::string src_file(argv[1]);
    std::string dst_file(argv[2]);
    // Original
    // int ASPECT = 1.5;
    // int SCALE = 1.0;
    // int src_size = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    // double R = src_size / 2.;
    // int dst_width = int(2.0*M_PI*R*SCALE/ASPECT);
    // int dst_height = int(R * SCALE);

    // HD (720p)
    int dst_width = 1280;
    int dst_height = 720;

    // Full-HD (1080p)
    // int dst_width = 1920;
    // int dst_height = 1080;

    int start_time = atoi(argv[3]);
    int end_time = atoi(argv[4]);
    int n_split = atoi(argv[5]);
    double angle_start = atof(argv[6]) * M_PI;
    double angle_end = atof(argv[7]) * M_PI;
    convert(src_file, dst_file,
            dst_width, dst_height,
            start_time, end_time,
            n_split,
            angle_start,
            angle_end,
            [](float progress){
        std::cout << progress << std::endl;
    });
    return 0;
}
*/
