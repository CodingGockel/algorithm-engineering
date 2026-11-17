#pragma once

#include <vector>
#include "point.h"

// Picks n points from `points` that are spread as evenly as possible:
// start with points[0], then repeatedly take the point farthest from all chosen ones.
// O(n * points.size()). Requires n <= points.size() (throws std::invalid_argument otherwise).
std::vector<point> farthest_point_sampling(const std::vector<point>& points, size_t n);
