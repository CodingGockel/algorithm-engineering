#include "stats.h"

#include <cmath>

stat_vector compute_stats(const std::vector<point>& points)
{
    const double n = static_cast<double>(points.size());

    double sum_x = 0.0;
    double sum_y = 0.0;
    for (const point& p : points)
    {
        sum_x += p.x;
        sum_y += p.y;
    }
    const double mean_x = sum_x / n;
    const double mean_y = sum_y / n;

    // two-pass variance for numerical stability
    double sq_x = 0.0;
    double sq_y = 0.0;
    for (const point& p : points)
    {
        sq_x += (p.x - mean_x) * (p.x - mean_x);
        sq_y += (p.y - mean_y) * (p.y - mean_y);
    }
    const double sd_x = std::sqrt(sq_x / (n - 1.0));
    const double sd_y = std::sqrt(sq_y / (n - 1.0));

    return {mean_x, mean_y, sd_x, sd_y};
}

const std::vector<std::string>& stat_names()
{
    static const std::vector<std::string> names = {"mean x", "mean y", "sd x", "sd y"};
    return names;
}

bool stats_equal(const stat_vector& a, const stat_vector& b, int decimals)
{
    if (a.size() != b.size())
    {
        return false;
    }
    const double factor = std::pow(10.0, decimals);
    for (size_t i = 0; i < a.size(); i++)
    {
        if (std::floor(a[i] * factor) != std::floor(b[i] * factor))
        {
            return false;
        }
    }
    return true;
}
