#pragma once

#include <vector>
#include "point.h"
#include "image.h"

// Converts pixel coordinates (y pointing down) to math coordinates (y pointing up).
std::vector<point> pixels_to_math(const std::vector<point>& pixels);

// Nearest-neighbour upscaling of a (masked) image by an integer factor.
image upscale_nearest(const image& src, int factor);

// Per-axis scale factors that were applied to the shape (new unit per old unit).
struct shape_fit_result
{
    double scale_x = 1.0;
    double scale_y = 1.0;
};

// Shifts and scales `shape` per axis so that its mean and sample standard deviation (ddof = 1)
// are exactly those of `data`. Throws std::invalid_argument if the shape has zero spread on an axis.
shape_fit_result fit_shape_to_data(std::vector<point>& shape, const std::vector<point>& data);
