#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <functional>
#include <opencv2/opencv.hpp>

#define SP360_LAMBDA_ANGLE (M_PI*107.0/180.0)
//#define SP360_LAMBDA_ANGLE (M_PI*90.0/180.0)

#define EQUIRECTANGULAR_PROJECTION 1
#define CENTRAL_PROJECTION 2

namespace SP360
{
    class Converter {
    public:
        unsigned int dst_width;
        unsigned int dst_height;

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

        void projectionType(int pt){ this->_projection_type = pt;};

        void   angleStart(double th){ this->_angle_start = th;};
        double angleStart() { return this->_angle_start; };
        void   angleEnd(double th){ this->_angle_end = th;};
        double angleEnd() { return this->_angle_end; };

        void   radiusStart(double r){ this->_radius_start = r;};
        double radiusStart(){ return this->_radius_start;};
        void   radiusEnd(double r){ this->_radius_end = r;};
        double radiusEnd(){ return this->_radius_end;};

        void   aspect(double a){ this->_aspect = a;};
        double aspect(){ return this->_aspect;};
        void   fov(double f){ this->_fov = f;};
        double fov(){ return this->_fov;};
        void   centerAngle(double f){ this->_center_angle = f;};
        double centerAngle(){ return this->_center_angle;};
        void   centerRadius(double f){ this->_center_radius = f;};
        double centerRadius(){ return this->_center_radius;};

        void splitX(int n){ this->_split_x = n;};
        void splitY(int n){ this->_split_y = n;};

        bool isOpened(){ return videoCapture.isOpened();}

        double totalFrame(){return videoCapture.get(cv::CAP_PROP_FRAME_COUNT);};
        double fps(){return videoCapture.get(cv::CAP_PROP_FPS);};

        void splitOrderRow(){this->_split_order_row = true;};
        void splitOrderCol(){this->_split_order_row = false;};

    private:
        void convertImage(cv::Mat src_img, cv::Mat dst_img);
        cv::Point calcOriginalPoint(cv::Point converted_pos, cv::MatSize converted_size,
                                    cv::MatSize original_size, int split_idx);

        cv::VideoCapture videoCapture;
        cv::Mat previewImage;
        unsigned long _start_frame, _end_frame;
        int _width, _height;
        int _split_x, _split_y;
        bool _split_order_row;

        int _projection_type;

        // Parameter for Equirectangular Projection
        double _radius_start, _radius_end;
        double _angle_start, _angle_end;

        // Parameter for Central Projection
        double _center_angle;
        double _center_radius;
        double _aspect;
        double _fov;
    };

    // TODO: Legacy function
    // cv::Point calcOriginalPoint(cv::Point converted_pos, cv::MatSize original_size, cv::MatSize converted_size,
    //     double angle_start, double angle_end, double radius_in, double radius_out, int n_split);
}
#endif // CONVERTER_H
