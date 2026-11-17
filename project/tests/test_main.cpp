// Unit tests: make test
#undef NDEBUG
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <set>

#include "date_parser.h"
#include "data_loader.h"
#include "stats.h"
#include "normalize.h"
#include "sampling.h"
#include "shape_fit.h"
#include "assignment.h"
#include "morph.h"

static bool near(double a, double b, double eps) { return std::fabs(a - b) <= eps; }

static void test_dates()
{
    assert(days_from_civil(1970, 1, 1) == 0);
    assert(days_from_civil(2015, 2, 17) == 16483);
    assert(days_from_civil(1871, 2, 1) == -36128);
    assert(days_from_civil(2000, 3, 1) == 11017);

    double d = 0;
    assert(parse_iso_date("2015-02-17\r", d) && d == 16483);
    assert(!parse_iso_date("127.83", d));
    assert(!parse_iso_date("2015-13-01", d));

    bool is_date = true;
    assert(parse_value(" 127.5 ", is_date) == 127.5 && !is_date);
    assert(parse_value("1970-01-02", is_date) == 1 && is_date);
    bool threw = false;
    try { parse_value("abc", is_date); } catch (const std::exception&) { threw = true; }
    assert(threw);
    std::cout << "dates ok" << std::endl;
}

static void test_loader_and_stats()
{
    csv_data data;
    bool x_is_date = false;
    std::string err;
    assert(load_series("resources/data/aapl_2015_2017.csv", data, x_is_date, err));
    assert(x_is_date);
    assert(data.data.size() == 506 && data.length == 506);

    // reference values computed with python (statistics.mean / stdev)
    const stat_vector s = compute_stats(data.data);
    assert(near(s[0], 16846.95, 0.01));
    assert(near(s[1], 112.96, 0.01));
    assert(near(s[2], 211.72, 0.01));
    assert(near(s[3], 11.24, 0.01));

    assert(stats_equal({1.234, 5.0}, {1.239, 5.009}, 2));
    assert(!stats_equal({1.234}, {1.241}, 2));
    std::cout << "loader + stats ok" << std::endl;
}

static void test_normalize()
{
    std::vector<point> pts = {{-5, 100}, {5, 300}, {0, 200}};
    const point_map m = fit_to_range(pts, 10, 90);
    const std::vector<point> n = apply_map(pts, m);
    assert(near(n[0].x, 10, 1e-12) && near(n[1].x, 90, 1e-12) && near(n[2].y, 50, 1e-12));
    const std::vector<point> back = invert_map(n, m);
    for (size_t i = 0; i < pts.size(); i++)
        assert(near(back[i].x, pts[i].x, 1e-9) && near(back[i].y, pts[i].y, 1e-9));
    std::cout << "normalize ok" << std::endl;
}

static void test_sampling()
{
    std::vector<point> grid;
    for (int y = 0; y < 30; y++)
        for (int x = 0; x < 30; x++) grid.push_back({double(x), double(y)});

    const std::vector<point> s = farthest_point_sampling(grid, 100);
    assert(s.size() == 100);
    std::set<std::pair<double, double>> unique;
    for (const point& p : s) unique.insert({p.x, p.y});
    assert(unique.size() == 100);
    std::cout << "sampling ok" << std::endl;
}

static void test_shape_fit()
{
    std::mt19937 rng(1);
    std::normal_distribution<double> g(0, 1);
    std::vector<point> data;
    std::vector<point> shape;
    for (int i = 0; i < 50; i++)
    {
        data.push_back({50 + 10 * g(rng), 40 + 3 * g(rng)});
        shape.push_back({200 + 80 * g(rng), -100 + 20 * g(rng)});
    }
    fit_shape_to_data(shape, data);
    const stat_vector a = compute_stats(data);
    const stat_vector b = compute_stats(shape);
    for (size_t i = 0; i < a.size(); i++) assert(near(a[i], b[i], 1e-9));
    std::cout << "shape fit ok" << std::endl;
}

static void test_hungarian()
{
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> u(0, 100);
    for (int n = 1; n <= 7; n++)
    {
        for (int rep = 0; rep < 20; rep++)
        {
            std::vector<double> cost(n * n);
            for (double& c : cost) c = u(rng);

            const std::vector<int> a = hungarian(cost, n);
            double got = 0;
            for (int i = 0; i < n; i++) got += cost[i * n + a[i]];

            std::vector<int> perm(n);
            std::iota(perm.begin(), perm.end(), 0);
            double best = 1e300;
            do
            {
                double c = 0;
                for (int i = 0; i < n; i++) c += cost[i * n + perm[i]];
                best = std::min(best, c);
            } while (std::next_permutation(perm.begin(), perm.end()));

            assert(near(got, best, 1e-9));
            std::set<int> cols(a.begin(), a.end());
            assert(static_cast<int>(cols.size()) == n);
        }
    }
    std::cout << "hungarian ok" << std::endl;
}

static void test_morph_keeps_stats()
{
    std::mt19937 rng(3);
    std::uniform_real_distribution<double> u(20, 80);
    std::vector<point> pts;
    std::vector<point> targets;
    for (int i = 0; i < 60; i++)
    {
        pts.push_back({u(rng), u(rng)});
        targets.push_back({u(rng), u(rng)});
    }
    fit_shape_to_data(targets, pts);
    targets = assign_targets(pts, targets);

    const stat_vector before = compute_stats(pts);
    const double dist_before = mean_target_distance(pts, targets);
    morph_params p;
    p.iterations = 200000;
    p.verbose = false;
    run_morph(pts, targets, p);
    assert(stats_equal(before, compute_stats(pts), 2));
    assert(mean_target_distance(pts, targets) < dist_before);
    std::cout << "morph ok" << std::endl;
}

int main()
{
    test_dates();
    test_loader_and_stats();
    test_normalize();
    test_sampling();
    test_shape_fit();
    test_hungarian();
    test_morph_keeps_stats();
    std::cout << "all tests passed" << std::endl;
    return 0;
}
