#include "converter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h>

namespace SP360
{
    using namespace cv;

    int Converter::open(std::string src_file)
    {
        this->src_file = src_file;
        videoCapture.open(src_file.c_str());
        videoCapture.set(CAP_PROP_POS_MSEC, 0);
        videoCapture >> previewImage;

        Mat tmp_img;
        videoCapture >> tmp_img;
        if( tmp_img.empty() ) {
            return 1;
        }
        cvtColor(tmp_img, previewImage, CV_BGR2RGBA );

        return 0;
    }

    int Converter::makeOriginalPreviewImage(unsigned char* dst_array, int width, int height, bool border)
    {
        if (border) {
            this->makeConvertBorderImage(this->src_file, dst_array, width, height, this->preview_time,
                this->angle_start, this->angle_end, this->radius_in, this->radius_out, this->n_split,
                1024, 256);
        } else {
            this->makeImage(this->src_file, dst_array, width, height, this->preview_time);
        }
        return 0;
    }

    int Converter::makeConvertedPreviewImage(unsigned char* dst_array, int width, int height)
    {
        this->makeConvertedImage(this->src_file, dst_array, width, height, this->preview_time,
                               this->angle_start, this->angle_end, this->radius_in, this->radius_out, this->n_split);
        return 0;
    }

    int Converter::convert(std::string filename, std::function<void(float)> progress_callback)
    {

        return Converter::convertMovie(this->src_file, filename, this->dst_width, this->dst_height,
            this->start_time, this->end_time, this->angle_start, this->angle_end,
            this->radius_in, this->radius_out, this->n_split, progress_callback);
    }

    // Legacy
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
        for (int j = 0; j < dst_img.rows; j++) {
            for (int i = 0; i < dst_img.cols; i++) {
                Point dst_point = Point(i, j);
                Point src_point = calcOriginalPoint(dst_point, src_img.size, dst_img.size,
                                                    angle_start, angle_end,
                                                    radius_in, radius_out,
                                                    n_split);
                int channels = src_img.channels();
                for (int c = 0; c < channels; c++) {
                    int dst_idx = channels * (dst_point.x + dst_point.y * dst_img.cols) + c;
                    int src_idx = channels * (src_point.x + src_point.y * src_img.cols) + c;
                    dst_img.data[dst_idx] = src_img.data[src_idx];
                }
            }
        }
    }


    int Converter::makeImage(std::string src_file, unsigned char* dst_array,
                  unsigned int dst_width, unsigned int dst_height,
                  unsigned int time)
    {
        using namespace cv;
        Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
        resize(previewImage, dst_img, dst_img.size());
        return 0;
    }


    int Converter::makeConvertBorderImage(std::string src_file, unsigned char* dst_array,
                               unsigned int dst_width, unsigned int dst_height,
                               unsigned int time,
                               double angle_start, double angle_end,
                               double radius_in, double radius_out,
                               int n_split,
                               int n_points_w, int n_points_h)
    {
        Converter::makeImage(src_file, dst_array, dst_width, dst_height, time);
        Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
        Mat bd_img(n_points_h, n_points_w, CV_8U);
        for (int s = 0; s < n_split; s++) {
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
                double split_angle_start = angle_start + (angle_end - angle_start) * s / n_split;
                double split_angle_end = split_angle_start + (angle_end - angle_start) / n_split;
                Point point = calcOriginalPoint(Point(i, j), dst_img.size, bd_img.size,
                                                split_angle_start, split_angle_end,
                                                radius_in, radius_out,
                                                1);
                int idx = point.x + point.y*dst_img.size[1];
                dst_img.data[idx*4+0] = 255;
                dst_img.data[idx*4+1] = 0;
                dst_img.data[idx*4+2] = 0;
            }
        }
        return 0;
    }


    int Converter::makeConvertedImage(std::string src_file, unsigned char* dst_array,
                           unsigned int dst_width, unsigned int dst_height,
                           unsigned int time,
                           double angle_start, double angle_end,
                           double radius_in, double radius_out,
                           int n_split)
    {
        using namespace cv;
        Mat dst_img(dst_height, dst_width, CV_8UC4, dst_array);
        convertImage(previewImage, dst_img, angle_start, angle_end, radius_in, radius_out, n_split);
        return 0;
    }


    int Converter::convertMovie(std::string src_file, std::string dst_file,
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
}
