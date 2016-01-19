#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/opencv.hpp>
#include <node.h>

using namespace v8;
using namespace cv;
using namespace std;

int get_converted_width(int src_width, int src_height, double scale)
{
    double R = src_width / 2.;
    int dst_width = int(2.*M_PI*R*scale);
    return dst_width;
}

int get_converted_height(int src_width, int src_height, double scale)
{
    double R = src_width / 2.;
    int dst_height = int(R * scale);
    return dst_height;
}

Mat convert_frame(Mat src_frame, double scale)
{
    int src_width = src_frame.cols;
    int src_height = src_frame.rows;
    double R = src_width / 2.;
    int dst_width = get_converted_width(src_width, src_height, scale);
    int dst_height = get_converted_height(src_width, src_height, scale);
    Mat dst_frame = Mat(dst_height, dst_width, src_frame.type());;
    double r, th, src_i, src_j;
    for (int i = 0; i < dst_width; i++) {
        for (int j = 0; j < dst_height; j++) {
            r = j / scale;
            th = i / scale / R;
            src_i = R - r * sin(th);
            src_j = R - r * cos(th);

            dst_frame.at<Vec3b>(Point(i,j)) = src_frame.at<Vec3b>(Point(src_i,src_j));
        }
    }
    return dst_frame;
}

//void convert(String src_file, String dst_file, int start_time, int end_time, QProgressBar* progressBar)
void convert(string src_file, string dst_file, int start_time, int end_time)
{
    double scale = 1.0;
    VideoCapture cap(src_file.c_str());
    double fps = cap.get(CAP_PROP_FPS);
    int src_width = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    int src_height = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    int fourcc = VideoWriter::fourcc('a','v','c','1');
    int dst_width = get_converted_width(src_width, src_height, scale);
    int dst_height = get_converted_height(src_width, src_height, scale);

    VideoWriter writer(dst_file.c_str(), fourcc, fps, Size(dst_width, dst_height));
    Mat frame, dst_frame;
    int i = 0;
    while( 1 )
    {
        i++;
        printf("%f\n", float(i)/fps);
        cap >> frame;
        if( i < start_time*fps)
            continue;
        if( frame.empty() || i > end_time*fps)
            break;
        dst_frame = convert_frame(frame, scale);
        writer << dst_frame;
    }
}

void Method(const FunctionCallbackInfo<Value>& args) {
    string src_file("/Users/maruyama/Desktop/SP360Converter/SP01_04.MP4");
    string dst_file("/Users/maruyama/Desktop/SP360Converter/test2.mp4");
    convert(src_file, dst_file, 0, 1);
}

void init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "convert", Method);
}
/*
int main(int argc, char const *argv[]) {
    String src(argv[1]);
    String dst(argv[2]);
    convert(src, dst, 0, 3);
    return 0;
}
*/
NODE_MODULE(addon, init)
