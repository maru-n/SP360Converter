#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <functional>

int convert(std::string src_file, std::string dst_file,
            int dst_width, int dst_height,
            int start_time, int end_time,
            int n_split,
            double start_angle, double end_angle,
            std::function<void(float)> callback);

#endif // CONVERTER_H
