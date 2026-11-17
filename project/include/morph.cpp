#include "morph.h"
#include "stats.h"

#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>

double ease_in_out_quad(double t)
{
    return t < 0.5 ? 2.0 * t * t : 1.0 - (-2.0 * t + 2.0) * (-2.0 * t + 2.0) / 2.0;
}

static double distance(const point& a, const point& b)
{
    return std::hypot(a.x - b.x, a.y - b.y);
}

double mean_target_distance(const std::vector<point>& points, const std::vector<point>& targets)
{
    double sum = 0.0;
    for (size_t i = 0; i < points.size(); i++)
    {
        sum += distance(points[i], targets[i]);
    }
    return sum / static_cast<double>(points.size());
}

morph_result run_morph(std::vector<point>& points, const std::vector<point>& targets, const morph_params& params)
{
    if (points.size() != targets.size())
    {
        throw std::invalid_argument("run_morph: points and targets must have the same size");
    }

    std::mt19937_64 rng(params.seed);
    std::uniform_int_distribution<size_t> pick(0, points.size() - 1);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    std::normal_distribution<double> gauss(0.0, 1.0);

    const stat_vector original = compute_stats(points);
    const long long iters = params.iterations;
    const long long report_every = iters >= 10 ? iters / 10 : 1;
    morph_result result;

    auto in_bounds = [&](const point& p) {
        return p.x > params.bounds_min.x && p.x < params.bounds_max.x &&
               p.y > params.bounds_min.y && p.y < params.bounds_max.y;
    };

    for (long long i = 0; i <= iters; i++)
    {
        const double progress_left = static_cast<double>(iters - i) / static_cast<double>(iters);
        const double temp = (params.max_temp - params.min_temp) * ease_in_out_quad(progress_left) + params.min_temp;

        const size_t k = pick(rng);
        const point old_p = points[k];
        const point& target = targets[k];
        const double old_dist = distance(old_p, target);
        const bool do_bad = uniform(rng) < temp;

        // propose a move (perturb() in the reference)
        bool found = false;
        point new_p = old_p;
        for (int attempt = 0; attempt < params.max_tries; attempt++)
        {
            new_p = {old_p.x + gauss(rng) * params.shake, old_p.y + gauss(rng) * params.shake};
            const double new_dist = distance(new_p, target);
            if ((new_dist < old_dist || new_dist < params.allowed_dist || do_bad) && in_bounds(new_p))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            result.rejected_proposal++;
            continue;
        }

        // keep the move only if the stats stay the same (is_error_still_ok() in the reference)
        points[k] = new_p;
        if (stats_equal(compute_stats(points), original, params.decimals))
        {
            result.accepted++;
        }
        else
        {
            points[k] = old_p;
            result.rejected_stats++;
        }

        if (params.verbose && i % report_every == 0)
        {
            std::cout << "  iter " << i << "/" << iters << "  temp " << temp
                      << "  mean dist " << mean_target_distance(points, targets) << std::endl;
        }
    }

    return result;
}
