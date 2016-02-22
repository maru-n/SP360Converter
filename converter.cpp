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
    Mat tmp_img;
    cvtColor(src_img, tmp_img, CV_BGR2RGBA );

    Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
    resize(tmp_img, dst_img, dst_img.size());

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
