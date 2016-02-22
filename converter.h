#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <functional>
#include <opencv2/opencv.hpp>

int makeThumbnail(std::string src_file, unsigned char* dst_array,
                  int dst_width, int dst_height,
                  int time);

int convertMovie(std::string src_file, std::string dst_file,
                 int dst_width, int dst_height,
                 int start_time, int end_time,
                 double angle_start, double angle_end,
                 double radius_in, double radius_out,
                 int n_split,
                 std::function<void(float)> callback);

#endif // CONVERTER_H
