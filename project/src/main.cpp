#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "constants.h"
#include "image.h"
#include "csv_data.h"
#include "data_loader.h"
#include "stats.h"
#include "normalize.h"
#include "sampling.h"
#include "shape_fit.h"
#include "assignment.h"
#include "morph.h"
#include "plot.h"

namespace
{
    constexpr double NORMALIZED_MIN = 10.0;     // data is mapped onto [10, 90]^2
    constexpr double NORMALIZED_MAX = 90.0;
    constexpr double BOUNDS_MARGIN = 5.0;       // free space around data + target for the morph
    constexpr long long ITERATIONS_PER_POINT = 2000;
    constexpr int MAX_IMAGE_SIDE = 16384;       // upscaling limit when the outline has too few pixels

    struct cli_args
    {
        std::string csv_path = "resources/data/aapl_2015_2017.csv";
        std::string image_path = "resources/images/baer.png";
        std::string out_dir = "build/output";
        morph_params morph;
        long long iterations = 0; // 0 = ITERATIONS_PER_POINT * N
    };

    void print_usage(const char* prog)
    {
        std::cout << "usage: " << prog << " [options]\n"
                  << "  --csv <path>           input data (2 columns, x may be an ISO date)\n"
                  << "  --image <path>         target shape (png)\n"
                  << "  --out <dir>            output directory (default build/output)\n"
                  << "  --iters <n>            iterations (default " << ITERATIONS_PER_POINT << " * #points)\n"
                  << "  --seed <n>             random seed (default 42)\n"
                  << "  --shake <x>            perturbation sigma (default 0.1)\n"
                  << "  --allowed-dist <x>     always accept moves this close to the target (default 0.5)\n"
                  << "  --decimals <n>         stats precision (default 2)\n"
                  << "  --quiet                no progress output\n";
    }

    cli_args parse_args(int argc, char** argv)
    {
        cli_args args;
        for (int i = 1; i < argc; i++)
        {
            const std::string a = argv[i];
            auto value = [&]() -> std::string {
                if (i + 1 >= argc) throw std::invalid_argument("missing value for " + a);
                return argv[++i];
            };

            if (a == "--csv") args.csv_path = value();
            else if (a == "--image") args.image_path = value();
            else if (a == "--out") args.out_dir = value();
            else if (a == "--iters") args.iterations = std::stoll(value());
            else if (a == "--seed") args.morph.seed = std::stoull(value());
            else if (a == "--shake") args.morph.shake = std::stod(value());
            else if (a == "--allowed-dist") args.morph.allowed_dist = std::stod(value());
            else if (a == "--decimals") args.morph.decimals = std::stoi(value());
            else if (a == "--quiet") args.morph.verbose = false;
            else if (a == "--help" || a == "-h") { print_usage(argv[0]); std::exit(0); }
            else throw std::invalid_argument("unknown argument: " + a);
        }
        return args;
    }

    // Loads the image, masks it and returns at least min_points outline pixels (upscaling if necessary).
    std::vector<point> load_outline(const std::string& path, size_t min_points, const std::string& debug_png)
    {
        image img;
        if (!load_image(path, img))
        {
            throw std::runtime_error("cannot load image: " + path);
        }
        mask_image(img);

        image mask = img;
        int factor = 1;
        while (true)
        {
            image work = mask;
            std::vector<point> outline;
            outline.reserve(static_cast<size_t>(work.width) * work.height);
            get_outline(work, outline);

            if (outline.empty())
            {
                throw std::runtime_error("image has no outline (fully black or fully white?): " + path);
            }
            if (outline.size() >= min_points)
            {
                save_image(debug_png, work);
                std::cout << "outline: " << outline.size() << " pixels"
                          << (factor > 1 ? " (image upscaled x" + std::to_string(factor) + ")" : "") << std::endl;
                return outline;
            }
            if (mask.width * 2 > MAX_IMAGE_SIDE || mask.height * 2 > MAX_IMAGE_SIDE)
            {
                throw std::runtime_error("outline has too few pixels (" + std::to_string(outline.size()) +
                                         ") even after upscaling");
            }
            mask = upscale_nearest(mask, 2);
            factor *= 2;
        }
    }

    void print_stats(const std::string& label, const stat_vector& s)
    {
        std::cout << "  " << std::left << std::setw(8) << label;
        for (size_t i = 0; i < s.size(); i++)
        {
            std::cout << stat_names()[i] << " = " << std::fixed << std::setprecision(4) << s[i] << "   ";
        }
        std::cout << std::defaultfloat << std::endl;
    }

    void write_csv(const std::string& path, const csv_data& data, bool x_is_date, const std::vector<point>& pts)
    {
        std::ofstream out(path);
        out << data.x_axis << (x_is_date ? "_days_since_1970" : "") << "," << data.y_axis << "\n";
        out << std::setprecision(12);
        for (const point& p : pts)
        {
            out << p.x << "," << p.y << "\n";
        }
    }

    double seconds_since(std::chrono::steady_clock::time_point t0)
    {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    }
}

int main(int argc, char** argv)
{
    try
    {
        cli_args args = parse_args(argc, argv);
        const auto t_start = std::chrono::steady_clock::now();

        // 1. data
        csv_data data;
        bool x_is_date = false;
        std::string error;
        if (!load_series(args.csv_path, data, x_is_date, error))
        {
            throw std::runtime_error(error);
        }
        const size_t n = data.data.size();
        if (n < 3)
        {
            throw std::runtime_error("need at least 3 data points, got " + std::to_string(n));
        }
        std::cout << "data: " << n << " points (" << data.x_axis << (x_is_date ? " as calendar days" : "")
                  << " / " << data.y_axis << ")" << std::endl;

        // 2. normalize
        const point_map norm = fit_to_range(data.data, NORMALIZED_MIN, NORMALIZED_MAX);
        std::vector<point> points = apply_map(data.data, norm);
        const std::vector<point> start = points;

        const std::string name = std::filesystem::path(args.csv_path).stem().string() + "__" +
                                 std::filesystem::path(args.image_path).stem().string();
        std::filesystem::create_directories(args.out_dir);
        const std::string prefix = (std::filesystem::path(args.out_dir) / name).string();

        // 3. outline -> n evenly spread target points
        std::vector<point> outline = load_outline(args.image_path, n, prefix + "_outline.png");
        std::vector<point> targets = pixels_to_math(farthest_point_sampling(outline, n));

        // 4. fit the shape to mean / sd of the data
        const shape_fit_result fit = fit_shape_to_data(targets, points);

        // 5. 1:1 assignment
        auto t0 = std::chrono::steady_clock::now();
        targets = assign_targets(points, targets);
        std::cout << "assignment (hungarian): " << seconds_since(t0) << " s" << std::endl;

        // 6. morph
        point bmin;
        point bmax;
        std::vector<point> both = points;
        both.insert(both.end(), targets.begin(), targets.end());
        bounding_box(both, bmin, bmax);
        args.morph.bounds_min = {bmin.x - BOUNDS_MARGIN, bmin.y - BOUNDS_MARGIN};
        args.morph.bounds_max = {bmax.x + BOUNDS_MARGIN, bmax.y + BOUNDS_MARGIN};
        args.morph.iterations = args.iterations > 0 ? args.iterations : ITERATIONS_PER_POINT * static_cast<long long>(n);

        const stat_vector stats_before = compute_stats(points);
        const double dist_before = mean_target_distance(points, targets);
        std::cout << "morph: " << args.morph.iterations << " iterations" << std::endl;

        t0 = std::chrono::steady_clock::now();
        const morph_result res = run_morph(points, targets, args.morph);
        const double morph_time = seconds_since(t0);

        // 7. results
        const stat_vector stats_after = compute_stats(points);
        const bool ok = stats_equal(stats_before, stats_after, args.morph.decimals);
        const std::vector<point> final_original = invert_map(points, norm);

        std::cout << "\nstats (original units):" << std::endl;
        print_stats("before", compute_stats(data.data));
        print_stats("after", compute_stats(final_original));
        std::cout << "stats equal (normalized, " << args.morph.decimals << " decimals): " << (ok ? "yes" : "NO") << std::endl;
        std::cout << "mean distance to target (normalized): " << dist_before << " -> "
                  << mean_target_distance(points, targets) << std::endl;
        std::cout << "accepted " << res.accepted << ", rejected (stats) " << res.rejected_stats
                  << ", no proposal " << res.rejected_proposal << std::endl;
        std::cout << "morph time " << morph_time << " s, total " << seconds_since(t_start) << " s" << std::endl;

        write_csv(prefix + ".csv", data, x_is_date, final_original);

        plot_options plot;
        plot.aspect_x = fit.scale_x;
        plot.aspect_y = fit.scale_y;
        if (!render_comparison_png(prefix + ".png", start, points, targets, plot))
        {
            throw std::runtime_error("cannot write " + prefix + ".png");
        }
        std::cout << "written: " << prefix << ".png, " << prefix << ".csv, " << prefix << "_outline.png" << std::endl;

        return ok ? 0 : 2;
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}
