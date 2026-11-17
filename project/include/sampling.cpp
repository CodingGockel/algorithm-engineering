#include "sampling.h"

#include <limits>
#include <stdexcept>

std::vector<point> farthest_point_sampling(const std::vector<point>& points, size_t n)
{
    if (n > points.size())
    {
        throw std::invalid_argument("farthest_point_sampling: n is larger than the number of points");
    }

    std::vector<point> chosen;
    chosen.reserve(n);
    if (n == 0)
    {
        return chosen;
    }

    // min_dist[i] = squared distance of points[i] to the nearest chosen point
    std::vector<double> min_dist(points.size(), std::numeric_limits<double>::infinity());
    size_t next = 0;

    for (size_t k = 0; k < n; k++)
    {
        const point c = points[next];
        chosen.push_back(c);

        double best = -1.0;
        size_t best_idx = 0;
        for (size_t i = 0; i < points.size(); i++)
        {
            const double dx = points[i].x - c.x;
            const double dy = points[i].y - c.y;
            const double d = dx * dx + dy * dy;
            if (d < min_dist[i])
            {
                min_dist[i] = d;
            }
            if (min_dist[i] > best)
            {
                best = min_dist[i];
                best_idx = i;
            }
        }
        next = best_idx;
    }

    return chosen;
}
