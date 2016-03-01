#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <functional>
#include <opencv2/opencv.hpp>

namespace SP360
{
    class Converter {
    public:
        unsigned int dst_width;
        unsigned int dst_height;
        double angle_start;
        double angle_end;
        double radius_start;
        double radius_end;
        int n_split;

        int n_points_w;
        int n_points_h;


        Converter();
        ~Converter();
        int open(std::string src_file);
        int makeOriginalPreviewImage(unsigned char* dst_array, int width, int height, bool border);
        int makeConvertedPreviewImage(unsigned char* dst_array, int width, int height);
        int convert(std::string filename, std::function<void(float)> progress_callback);

        int width() { return _width;};
        int height() { return _height;};

        double startTimeMsec() { return _start_time_msec; };
        void startTimeMsec(double ms){ this->_start_time_msec = ms; };
        double endTimeMsec() { return _end_time_msec; };
        void endTimeMsec(double ms){ this->_end_time_msec = ms; };

        bool isOpened(){ return videoCapture.isOpened();}

        double totalMsec(){return 1000.0 * this->totalFrame() / this->fps();};
        double totalFrame(){return videoCapture.get(cv::CAP_PROP_FRAME_COUNT);};
        double fps(){return videoCapture.get(cv::CAP_PROP_FPS);};

    private:
        void convertImage(cv::Mat src_img, cv::Mat dst_img);

        cv::VideoCapture videoCapture;
        cv::Mat previewImage;
        double _start_time_msec, _end_time_msec, _preview_time_msec;
        int _width, _height;
    };

    // TODO: Legacy function
    cv::Point calcOriginalPoint(cv::Point converted_pos, cv::MatSize original_size, cv::MatSize converted_size,
        double angle_start, double angle_end, double radius_in, double radius_out, int n_split);
}
#endif // CONVERTER_H
