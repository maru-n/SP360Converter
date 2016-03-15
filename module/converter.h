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

        unsigned long    startFrame() {return _start_frame;}
        void             startFrame(unsigned long f){ this->_start_frame = f;};
        unsigned long    endFrame() {return _end_frame;}
        void             endFrame(unsigned long f){ this->_end_frame = f; };

        void   angleStart(double th){ this->_angle_start = th;};
        double angleStart() { return this->_angle_start; };
        void   angleEnd(double th){ this->_angle_end = th;};
        double angleEnd() { return this->_angle_end; };

        bool isOpened(){ return videoCapture.isOpened();}

        double totalFrame(){return videoCapture.get(cv::CAP_PROP_FRAME_COUNT);};
        double fps(){return videoCapture.get(cv::CAP_PROP_FPS);};

    private:
        //double frameToMsec(int f){return }
        void convertImage(cv::Mat src_img, cv::Mat dst_img);

        cv::VideoCapture videoCapture;
        cv::Mat previewImage;
        unsigned long _start_frame, _end_frame;
        int _width, _height;
        double _angle_start, _angle_end;
    };

    // TODO: Legacy function
    cv::Point calcOriginalPoint(cv::Point converted_pos, cv::MatSize original_size, cv::MatSize converted_size,
        double angle_start, double angle_end, double radius_in, double radius_out, int n_split);
}
#endif // CONVERTER_H
