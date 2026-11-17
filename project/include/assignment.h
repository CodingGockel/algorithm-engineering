#pragma once

#include <vector>
#include "point.h"

// Hungarian algorithm (Kuhn-Munkres with potentials), O(n^3).
// cost is a row-major n x n matrix. Returns assignment[row] = column with minimal total cost.
std::vector<int> hungarian(const std::vector<double>& cost, int n);

// Assigns every point to exactly one target, minimising the sum of squared distances.
// Returns targets reordered so that result[i] belongs to points[i].
// Requires points.size() == targets.size().
std::vector<point> assign_targets(const std::vector<point>& points, const std::vector<point>& targets);
