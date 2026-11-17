#pragma once

#include <string>
#include <vector>
#include "point.h"

// The statistics kept constant during the morph.
// To add a new measure: append it in compute_stats() and stat_names().
using stat_vector = std::vector<double>;

// [mean_x, mean_y, sd_x, sd_y]; sd uses ddof = 1 (sample standard deviation, like pandas).
stat_vector compute_stats(const std::vector<point>& points);

const std::vector<std::string>& stat_names();

// Same check as the reference implementation: floor(v * 10^decimals) must be equal for every stat.
bool stats_equal(const stat_vector& a, const stat_vector& b, int decimals);
