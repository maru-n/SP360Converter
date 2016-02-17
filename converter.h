#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <functional>

void convert(std::string src_file, std::string dst_file,
             int dst_width, int dst_height,
             int start_time, int end_time,
             int n_split,
             double start_theta, double end_theta,
             std::function<void(float)> callback);

#endif // CONVERTER_H
