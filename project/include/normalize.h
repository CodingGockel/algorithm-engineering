#pragma once

#include <vector>
#include "point.h"

// Linear map v -> v * scale + offset for one axis.
struct axis_map
{
    double scale = 1.0;
    double offset = 0.0;

    double apply(double v) const { return v * scale + offset; }
    double invert(double v) const { return (v - offset) / scale; }
};

struct point_map
{
    axis_map x;
    axis_map y;

    point apply(const point& p) const { return {x.apply(p.x), y.apply(p.y)}; }
    point invert(const point& p) const { return {x.invert(p.x), y.invert(p.y)}; }
};

// Map whose image of the points' bounding box is [lo, hi] on both axes.
// Throws std::invalid_argument if an axis has zero range.
point_map fit_to_range(const std::vector<point>& points, double lo, double hi);

std::vector<point> apply_map(const std::vector<point>& points, const point_map& map);
std::vector<point> invert_map(const std::vector<point>& points, const point_map& map);

// Bounding box of a point set.
void bounding_box(const std::vector<point>& points, point& min, point& max);
