#include "shape_fit.h"
#include "stats.h"

#include <stdexcept>

std::vector<point> pixels_to_math(const std::vector<point>& pixels)
{
    std::vector<point> out;
    out.reserve(pixels.size());
    for (const point& p : pixels)
    {
        out.push_back({p.x, -p.y});
    }
    return out;
}

image upscale_nearest(const image& src, int factor)
{
    image dst;
    dst.width = src.width * factor;
    dst.height = src.height * factor;
    dst.has_alpha = src.has_alpha;
    dst.pixels.resize(static_cast<size_t>(dst.width) * dst.height);

    for (int y = 0; y < dst.height; y++)
    {
        for (int x = 0; x < dst.width; x++)
        {
            dst.at(x, y) = src.at(x / factor, y / factor);
        }
    }
    return dst;
}

shape_fit_result fit_shape_to_data(std::vector<point>& shape, const std::vector<point>& data)
{
    const stat_vector s = compute_stats(shape); // [mean_x, mean_y, sd_x, sd_y]
    const stat_vector d = compute_stats(data);

    if (!(s[2] > 0.0) || !(s[3] > 0.0))
    {
        throw std::invalid_argument("shape has zero spread on one axis (e.g. a single straight line), cannot fit");
    }

    shape_fit_result result;
    result.scale_x = d[2] / s[2];
    result.scale_y = d[3] / s[3];

    for (point& p : shape)
    {
        p.x = (p.x - s[0]) * result.scale_x + d[0];
        p.y = (p.y - s[1]) * result.scale_y + d[1];
    }
    return result;
}
