#include "converter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;


void convertImage(cv::Mat src_img, cv::Mat dst_img,
                  double angle_start, double angle_end,
                  double radius_in, double radius_out,
                  int n_split)
{
    double R = src_img.cols / 2.;
    int w = double(dst_img.cols);
    int h = double(dst_img.rows);
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
            dst_img.at<Vec3b>(Point(i, j)) = src_img.at<Vec3b>(Point(src_i,src_j));
        }
    }
}


void print_info(const cv::Mat& mat)
{
    using namespace std;

    cout << "type: " << (
        mat.type() == CV_8UC3 ? "CV_8UC3" :
        mat.type() == CV_16SC1 ? "CV_16SC1" :
        mat.type() == CV_64FC2 ? "CV_64FC2" :
        "other"
        ) << endl;
    cout << "depth: " << (
        mat.depth() == CV_8U ? "CV_8U" :
        mat.depth() == CV_16S ? "CV_16S" :
        mat.depth() == CV_64F ? "CV_64F" :
        "other"
        ) << endl;
    cout << "channels: " << mat.channels() << endl;
    cout << "continuous: " <<
        (mat.isContinuous() ? "true" : "false")<< endl;
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
