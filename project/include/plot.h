#pragma once

#include <string>
#include <vector>
#include "point.h"

struct plot_options
{
    int panel_size = 800;    // longer side of one panel in pixels
    int margin = 30;         // inner padding of a panel in pixels
    // Display aspect correction: x is drawn divided by aspect_x, y divided by aspect_y.
    // Pass the per-axis scale factors of the shape fit so the target shape looks undistorted.
    double aspect_x = 1.0;
    double aspect_y = 1.0;
};

// Writes a PNG with two panels sharing the same coordinate system:
//   left:  `start` as a line chart in the given (time) order
//   right: `final` as a scatter plot, `targets` as light grey points behind it
// Points are coloured by their index (i.e. by date for time series).
bool render_comparison_png(const std::string& path,
                           const std::vector<point>& start,
                           const std::vector<point>& final,
                           const std::vector<point>& targets,
                           const plot_options& options = {});
