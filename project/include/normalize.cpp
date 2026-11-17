#include "normalize.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

void bounding_box(const std::vector<point>& points, point& min, point& max)
{
    min = {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    max = {-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    for (const point& p : points)
    {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
    }
}

static axis_map fit_axis(double v_min, double v_max, double lo, double hi, const char* name)
{
    if (!(v_max > v_min))
    {
        throw std::invalid_argument(std::string("axis ") + name + " has zero range, cannot normalize");
    }
    axis_map m;
    m.scale = (hi - lo) / (v_max - v_min);
    m.offset = lo - v_min * m.scale;
    return m;
}

point_map fit_to_range(const std::vector<point>& points, double lo, double hi)
{
    point min;
    point max;
    bounding_box(points, min, max);
    return {fit_axis(min.x, max.x, lo, hi, "x"), fit_axis(min.y, max.y, lo, hi, "y")};
}

std::vector<point> apply_map(const std::vector<point>& points, const point_map& map)
{
    std::vector<point> out;
    out.reserve(points.size());
    for (const point& p : points) out.push_back(map.apply(p));
    return out;
}

std::vector<point> invert_map(const std::vector<point>& points, const point_map& map)
{
    std::vector<point> out;
    out.reserve(points.size());
    for (const point& p : points) out.push_back(map.invert(p));
    return out;
}
