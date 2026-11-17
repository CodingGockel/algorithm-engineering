#pragma once

#include <cstdint>
#include <vector>
#include "point.h"

// Parameters of the simulated-annealing morph (defaults follow the reference implementation
// https://github.com/khuyentran1401/same-stats-different-graphs, except allowed_dist).
struct morph_params
{
    long long iterations = 1000000;
    double shake = 0.1;          // sigma of the gaussian perturbation
    double max_temp = 0.4;       // probability of accepting a worse move at the start
    double min_temp = 0.0;       // ... and at the end
    double allowed_dist = 0.5;   // moves ending this close to the target are always ok
    int decimals = 2;            // stats must stay equal up to this many decimals (floored)
    int max_tries = 1000;        // proposal attempts per iteration (reference: unbounded)
    std::uint64_t seed = 42;
    point bounds_min{0.0, 0.0};  // points must stay strictly inside the bounds
    point bounds_max{100.0, 100.0};
    bool verbose = true;         // print progress
};

struct morph_result
{
    long long accepted = 0;           // moves that kept the stats and were applied
    long long rejected_stats = 0;     // proposals rejected because the stats changed
    long long rejected_proposal = 0;  // iterations without a valid proposal after max_tries
};

double ease_in_out_quad(double t);

// Moves points[i] towards targets[i] while keeping compute_stats(points) equal (to `decimals`).
morph_result run_morph(std::vector<point>& points, const std::vector<point>& targets, const morph_params& params);

// Mean euclidean distance between points[i] and targets[i].
double mean_target_distance(const std::vector<point>& points, const std::vector<point>& targets);
