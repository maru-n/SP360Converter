#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <functional>
#include <opencv2/opencv.hpp>

namespace SP360
{
    class Converter {
    public:
        unsigned int start_time;
        unsigned int end_time;
        unsigned int dst_width;
        unsigned int dst_height;
        double angle_start;
        double angle_end;
        double radius_in;
        double radius_out;
        int n_split;

        int n_points_w;
        int n_points_h;
        unsigned int preview_time;

        Converter();
        int open(std::string src_file);
        int makeOriginalPreviewImage(unsigned char* dst_array, int width, int height, bool border);
        int makeConvertedPreviewImage(unsigned char* dst_array, int width, int height);
        int convert(std::string filename, std::function<void(float)> progress_callback);

        int width(){ return _width;};
        int height(){ return _height;};

    private:
        cv::VideoCapture videoCapture;
        cv::Mat previewImage;

        void convertImage(cv::Mat src_img, cv::Mat dst_img);

        int _width, _height;
    };

    // TODO: Legacy function
    cv::Point calcOriginalPoint(cv::Point converted_pos, cv::MatSize original_size, cv::MatSize converted_size,
        double angle_start, double angle_end, double radius_in, double radius_out, int n_split);
}
#endif // CONVERTER_H
