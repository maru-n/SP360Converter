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
        this->_split_order_row = true;
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
        for (int s = 0; s < _split_x*_split_y; s++) {
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
                double r_s = _radius_start;
                double r_e = _radius_end;
                double th_s = _angle_start + s * (_angle_end - _angle_start) / (_split_x * _split_y);
                double th_e = _angle_start + (s+1) * (_angle_end - _angle_start) / (_split_x * _split_y);
                Point point = calcOriginalPoint(Point(i, j), bd_img.size, dst_img.size, r_s, th_s, r_e, th_e);
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
        Mat dst_img_window(dst_img.rows/_split_y, dst_img.cols/_split_x, CV_8U);
        for (int sx = 0; sx < _split_x; sx++) {
            for (int sy = 0; sy < _split_y; sy++) {
                int x_offset = dst_img_window.cols * sx;
                int y_offset = dst_img_window.rows * sy;
                int s;
                if (_split_order_row) {
                    s = sy + sx * _split_y;
                } else {
                    s = sx + sy * _split_x;
                }
                for (int j = 0; j < dst_img_window.rows; j++) {
                    for (int i = 0; i < dst_img_window.cols; i++) {
                        Point dst_point = Point(i,j);
                        double r_s = _radius_start;
                        double r_e = _radius_end;
                        double th_s = _angle_start + s * (_angle_end - _angle_start) / (_split_x*_split_y);
                        double th_e = _angle_start + (s+1) * (_angle_end - _angle_start) / (_split_x*_split_y);
                        Point src_point = calcOriginalPoint(dst_point, dst_img_window.size, src_img.size, r_s, th_s, r_e, th_e);
                        for (int c = 0; c < channels; c++) {
                            int dst_idx = channels * (dst_point.x+x_offset + (dst_point.y+y_offset) * dst_img.cols) + c;
                            int src_idx = channels * (src_point.x + src_point.y * src_img.cols) + c;
                            dst_img.data[dst_idx] = src_img.data[src_idx];
                        }
                    }
                }
            }
        }
    }

    Point calcOriginalPointNormal(Point converted_pos, MatSize converted_size,
                                  MatSize original_size,
                                  double start_r, double start_th,
                                  double end_r, double end_th)
    {
        double R = original_size[1] / 2.0;
        double th = start_th + converted_pos.x * (end_th - start_th) / converted_size[1];
        double r  = start_r + converted_pos.y * (end_r - start_r) / converted_size[0];
        double i = R + R * r * cos(th);
        double j = R - R * r * sin(th);
        return Point(i, j);
    }

    Point Converter::calcOriginalPoint(Point converted_pos, MatSize converted_size,
                                       MatSize original_size,
                                       double start_r, double start_th,
                                       double end_r, double end_th)
    {
        Point res = calcOriginalPointNormal(converted_pos, converted_size, original_size, start_r, start_th, end_r, end_th);
        return res;
    }
}
