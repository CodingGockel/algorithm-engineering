#include "assignment.h"

#include <limits>
#include <stdexcept>

std::vector<int> hungarian(const std::vector<double>& cost, int n)
{
    // Classic O(n^3) formulation with 1-based indices; column 0 is a virtual start column.
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> u(n + 1, 0.0);   // row potentials
    std::vector<double> v(n + 1, 0.0);   // column potentials
    std::vector<int> match(n + 1, 0);    // match[col] = row
    std::vector<int> way(n + 1, 0);
    std::vector<double> minv(n + 1);
    std::vector<char> used(n + 1);

    for (int row = 1; row <= n; row++)
    {
        match[0] = row;
        int j0 = 0;
        std::fill(minv.begin(), minv.end(), INF);
        std::fill(used.begin(), used.end(), 0);

        do
        {
            used[j0] = 1;
            const int i0 = match[j0];
            const double* cost_row = &cost[static_cast<size_t>(i0 - 1) * n];
            double delta = INF;
            int j1 = 0;

            for (int j = 1; j <= n; j++)
            {
                if (used[j]) continue;
                const double cur = cost_row[j - 1] - u[i0] - v[j];
                if (cur < minv[j])
                {
                    minv[j] = cur;
                    way[j] = j0;
                }
                if (minv[j] < delta)
                {
                    delta = minv[j];
                    j1 = j;
                }
            }

            for (int j = 0; j <= n; j++)
            {
                if (used[j])
                {
                    u[match[j]] += delta;
                    v[j] -= delta;
                }
                else
                {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (match[j0] != 0);

        do
        {
            const int j1 = way[j0];
            match[j0] = match[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    std::vector<int> assignment(n);
    for (int j = 1; j <= n; j++)
    {
        assignment[match[j] - 1] = j - 1;
    }
    return assignment;
}

std::vector<point> assign_targets(const std::vector<point>& points, const std::vector<point>& targets)
{
    if (points.size() != targets.size())
    {
        throw std::invalid_argument("assign_targets: points and targets must have the same size");
    }

    const int n = static_cast<int>(points.size());
    std::vector<double> cost(static_cast<size_t>(n) * n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            const double dx = points[i].x - targets[j].x;
            const double dy = points[i].y - targets[j].y;
            cost[static_cast<size_t>(i) * n + j] = dx * dx + dy * dy;
        }
    }

    const std::vector<int> assignment = hungarian(cost, n);
    std::vector<point> ordered(n);
    for (int i = 0; i < n; i++)
    {
        ordered[i] = targets[assignment[i]];
    }
    return ordered;
}
