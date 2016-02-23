#include "converter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;


Point calcOriginalPoint(Point converted_pos,
                        MatSize original_size, MatSize converted_size,
                        double angle_start, double angle_end,
                        double radius_in, double radius_out,
                        int n_split)
{
    double R = original_size[0] / 2.0;
    int w = converted_size[1];
    int h = converted_size[0];
    int pan_w = w * n_split;
    int pan_h = h / n_split;
    int split_row = converted_pos.y * n_split / h;
    double pan_i = converted_pos.x + w * split_row;
    double pan_j = converted_pos.y - h * split_row / n_split;
    double r = R * (radius_in + (radius_out - radius_in) * pan_j / pan_h);
    double th = angle_start + (angle_end - angle_start) * pan_i / pan_w;
    int src_i = R + r * cos(th);
    int src_j = R - r * sin(th);
    return Point(src_i,src_j);
}


void convertImage(cv::Mat src_img, cv::Mat dst_img,
                  double angle_start, double angle_end,
                  double radius_in, double radius_out,
                  int n_split)
{
    for (int j = 0; j < dst_img.size[0]; j++) {
        for (int i = 0; i < dst_img.size[1]; i++) {
            Point dst_point = Point(i, j);
            Point src_point = calcOriginalPoint(dst_point, src_img.size, dst_img.size,
                                                angle_start, angle_end,
                                                radius_in, radius_out,
                                                n_split);
            dst_img.at<Vec3b>(Point(i, j)) = src_img.at<Vec3b>(src_point);
        }
    }
}


int makeImage(std::string src_file, unsigned char* dst_array,
              unsigned int dst_width, unsigned int dst_height,
              unsigned int time)
{
    using namespace cv;

    VideoCapture cap(src_file.c_str());
    cap.set(CAP_PROP_POS_MSEC, time);
    Mat src_img;
    cap >> src_img;
    if( src_img.empty() ) {
        return 1;
    }
    Mat tmp_img;
    cvtColor(src_img, tmp_img, CV_BGR2RGBA );

    Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
    resize(tmp_img, dst_img, dst_img.size());

    return 0;
}


int makeConvertBorderImage(std::string src_file, unsigned char* dst_array,
                           unsigned int dst_width, unsigned int dst_height,
                           unsigned int time,
                           double angle_start, double angle_end,
                           double radius_in, double radius_out,
                           int n_split,
                           int n_points_w, int n_points_h)
{
    makeImage(src_file, dst_array, dst_width, dst_height, time);
    Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
    Mat bd_img(n_points_h, n_points_w, CV_8U);

    for (int n = 0; n < bd_img.size[1]*2 + bd_img.size[0]*2 - 4; n++) {
        int i, j;
        if (n < bd_img.size[1]) {
            i = n;
            j = 0;
        } else if (n < bd_img.size[1] + bd_img.size[0] - 1) {
            i = bd_img.size[1] - 1;
            j = n - bd_img.size[1] + 1;
        } else if (n < bd_img.size[1]*2 + bd_img.size[0] - 2) {
            i = n - bd_img.size[0] - bd_img.size[1] + 2;
            j = bd_img.size[0] - 1;
        } else {
            i = 0;
            j = n - bd_img.size[0] - bd_img.size[1]*2 + 3;
        }
        Point point = calcOriginalPoint(Point(i, j), dst_img.size, bd_img.size,
                                        angle_start, angle_end,
                                        radius_in, radius_out,
                                        n_split);
        int idx = point.x + point.y*dst_img.size[1];
        dst_img.data[idx*4+0] = 255;
        dst_img.data[idx*4+1] = 0;
        dst_img.data[idx*4+2] = 0;
    }

    for (int i = 0; i < bd_img.size[1] * (n_split-1); i++) {
        int j = (1 + i / bd_img.size[1]) * bd_img.size[0] / n_split;
        Point point = calcOriginalPoint(Point(i, j), dst_img.size, bd_img.size,
                                        angle_start, angle_end,
                                        radius_in, radius_out,
                                        n_split);
        int idx = point.x + point.y*dst_img.size[1];
        dst_img.data[idx*4+0] = 255;
        dst_img.data[idx*4+1] = 0;
        dst_img.data[idx*4+2] = 0;

        point = calcOriginalPoint(Point(i, j-1), dst_img.size, bd_img.size,
                                        angle_start, angle_end,
                                        radius_in, radius_out,
                                        n_split);
        idx = point.x + point.y*dst_img.size[1];
        dst_img.data[idx*4+0] = 255;
        dst_img.data[idx*4+1] = 0;
        dst_img.data[idx*4+2] = 0;
    }
    return 0;
}



int makeConvertedImage(std::string src_file, unsigned char* dst_array,
                       unsigned int dst_width, unsigned int dst_height,
                       unsigned int time,
                       double angle_start, double angle_end,
                       double radius_in, double radius_out,
                       int n_split)
{
    using namespace cv;

    VideoCapture cap(src_file.c_str());
    cap.set(CAP_PROP_POS_MSEC, time);
    Mat src_img;
    cap >> src_img;
    if( src_img.empty() ) {
        return 1;
    }
    Mat tmp_img(dst_height, dst_width, src_img.type());
    convertImage(src_img, tmp_img, angle_start, angle_end, radius_in, radius_out, n_split);

    Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
    cvtColor(tmp_img, dst_img, CV_BGR2RGBA);

    return 0;
}


int convertMovie(std::string src_file, std::string dst_file,
                 unsigned int dst_width, unsigned int dst_height,
                 unsigned int start_time, unsigned int end_time,
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
        convertImage(frame, new_frame, angle_start, angle_end, radius_in, radius_out, n_split);
        writer << new_frame;
        float progress = (current_pos - start_time) / (end_time - start_time);
        callback(progress);
    }
    return 0;
}
