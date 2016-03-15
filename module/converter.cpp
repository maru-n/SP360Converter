#include "converter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h>

namespace SP360
{
    using namespace cv;

    Converter::Converter()
    {
        this->n_points_w = 1024;
        this->n_points_h = 225;
    }

    Converter::~Converter() {}

    int Converter::open(std::string src_file)
    {
        videoCapture.open(src_file);
        this->_width = videoCapture.get(CAP_PROP_FRAME_WIDTH);
        this->_height = videoCapture.get(CAP_PROP_FRAME_HEIGHT);

        videoCapture.set(CAP_PROP_POS_FRAMES, 0);
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
        if (!this->isOpened()) { return -1; }

        Mat dst_img(height, width, CV_8UC4, dst_array);
        resize(previewImage, dst_img, dst_img.size());

        if (!border) { return 0; }

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
                double split_angle_start = _angle_start + (_angle_end - _angle_start) * s / n_split;
                double split_angle_end = split_angle_start + (_angle_end - _angle_start) / n_split;
                Point point = calcOriginalPoint(Point(i, j), dst_img.size, bd_img.size,
                                split_angle_start, split_angle_end,
                                radius_start, radius_end,
                                1);
                int idx = point.x + point.y*dst_img.size[1];
                dst_img.data[idx*4+0] = 255;
                dst_img.data[idx*4+1] = 0;
                dst_img.data[idx*4+2] = 0;
            }
        }

        return 0;
    }

    int Converter::makeConvertedPreviewImage(unsigned char* dst_array, int width, int height)
    {
        if (!this->isOpened()) { return -1; }
        Mat dst_img(height, width, CV_8UC4, dst_array);
        convertImage(previewImage, dst_img);
        return 0;
    }

    int Converter::convert(std::string filename, std::function<void(float)> progress_callback)
    {
        if (!this->isOpened()) { return -1; }
        int fourcc = static_cast<int>(videoCapture.get(CV_CAP_PROP_FOURCC));
        VideoWriter writer(filename, fourcc, this->fps(), Size(dst_width, dst_height));

        videoCapture.set(CAP_PROP_POS_FRAMES, _start_frame);
        Mat frame;
        while (1) {
            double current_frame = videoCapture.get(CAP_PROP_POS_FRAMES);
            // std::cout << "fram:" << videoCapture.get(CAP_PROP_POS_FRAMES)
            // << " / msec:" << videoCapture.get(CAP_PROP_POS_MSEC) << std::endl;
            if (current_frame > _end_frame) {
                break;
            }
            videoCapture >> frame;
            if( frame.empty() )
                return -1;
            Mat new_frame = Mat(dst_height, dst_width, frame.type());
            convertImage(frame, new_frame);
            writer << new_frame;
            float progress = (current_frame - _start_frame) / (_end_frame - _start_frame);
            progress_callback(progress);
        }
        return 0;
    }

    void Converter::convertImage(cv::Mat src_img, cv::Mat dst_img)
    {
        int channels = src_img.channels();
        for (int j = 0; j < dst_img.rows; j++) {
            for (int i = 0; i < dst_img.cols; i++) {
                Point dst_point = Point(i, j);
                Point src_point = calcOriginalPoint(dst_point, src_img.size, dst_img.size,
                                    _angle_start, _angle_end,
                                    radius_start, radius_end,
                                    n_split);
                for (int c = 0; c < channels; c++) {
                    int dst_idx = channels * (dst_point.x + dst_point.y * dst_img.cols) + c;
                    int src_idx = channels * (src_point.x + src_point.y * src_img.cols) + c;
                    dst_img.data[dst_idx] = src_img.data[src_idx];
                }
            }
        }
    }

    // TODO: Legacy function
    Point calcOriginalPoint(Point converted_pos,
                            MatSize original_size, MatSize converted_size,
                            double angle_start, double angle_end,
                            double radius_start, double radius_end,
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
        double r = R * (radius_start + (radius_end - radius_start) * pan_j / pan_h);
        double th = angle_start + (angle_end - angle_start) * pan_i / pan_w;
        int src_i = R + r * cos(th);
        int src_j = R - r * sin(th);
        return Point(src_i,src_j);
    }
}