#include "plot.h"
#include "normalize.h"
#include "stb_image_write.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace
{
    struct rgb
    {
        std::uint8_t r, g, b;
    };

    struct canvas
    {
        int width;
        int height;
        std::vector<std::uint8_t> data;

        canvas(int w, int h) : width(w), height(h), data(static_cast<size_t>(w) * h * 3, 255) {}

        void blend(int x, int y, rgb c, double alpha = 1.0)
        {
            if (x < 0 || y < 0 || x >= width || y >= height) return;
            std::uint8_t* px = &data[(static_cast<size_t>(y) * width + x) * 3];
            px[0] = static_cast<std::uint8_t>(px[0] + (c.r - px[0]) * alpha);
            px[1] = static_cast<std::uint8_t>(px[1] + (c.g - px[1]) * alpha);
            px[2] = static_cast<std::uint8_t>(px[2] + (c.b - px[2]) * alpha);
        }

        void fill_circle(double cx, double cy, double radius, rgb c, double alpha = 1.0)
        {
            const int x0 = static_cast<int>(std::floor(cx - radius));
            const int x1 = static_cast<int>(std::ceil(cx + radius));
            const int y0 = static_cast<int>(std::floor(cy - radius));
            const int y1 = static_cast<int>(std::ceil(cy + radius));
            for (int y = y0; y <= y1; y++)
            {
                for (int x = x0; x <= x1; x++)
                {
                    const double dx = x - cx;
                    const double dy = y - cy;
                    if (dx * dx + dy * dy <= radius * radius) blend(x, y, c, alpha);
                }
            }
        }

        // Bresenham line, thickened by drawing small discs
        void line(double xa, double ya, double xb, double yb, rgb c, double thickness = 1.0)
        {
            int x0 = static_cast<int>(std::lround(xa));
            int y0 = static_cast<int>(std::lround(ya));
            const int x1 = static_cast<int>(std::lround(xb));
            const int y1 = static_cast<int>(std::lround(yb));
            const int dx = std::abs(x1 - x0);
            const int dy = -std::abs(y1 - y0);
            const int sx = x0 < x1 ? 1 : -1;
            const int sy = y0 < y1 ? 1 : -1;
            int err = dx + dy;
            while (true)
            {
                if (thickness <= 1.0) blend(x0, y0, c);
                else fill_circle(x0, y0, thickness / 2.0, c);
                if (x0 == x1 && y0 == y1) break;
                const int e2 = 2 * err;
                if (e2 >= dy) { err += dy; x0 += sx; }
                if (e2 <= dx) { err += dx; y0 += sy; }
            }
        }

        void rect(int x0, int y0, int x1, int y1, rgb c)
        {
            line(x0, y0, x1, y0, c);
            line(x1, y0, x1, y1, c);
            line(x1, y1, x0, y1, c);
            line(x0, y1, x0, y0, c);
        }
    };

    // viridis approximation, t in [0, 1]
    rgb viridis(double t)
    {
        static const rgb stops[] = {{68, 1, 84}, {59, 82, 139}, {33, 145, 140}, {94, 201, 98}, {253, 231, 37}};
        t = std::clamp(t, 0.0, 1.0) * 4.0;
        const int i = std::min(static_cast<int>(t), 3);
        const double f = t - i;
        const rgb a = stops[i];
        const rgb b = stops[i + 1];
        return {static_cast<std::uint8_t>(a.r + (b.r - a.r) * f),
                static_cast<std::uint8_t>(a.g + (b.g - a.g) * f),
                static_cast<std::uint8_t>(a.b + (b.b - a.b) * f)};
    }
}

bool render_comparison_png(const std::string& path,
                           const std::vector<point>& start,
                           const std::vector<point>& final,
                           const std::vector<point>& targets,
                           const plot_options& options)
{
    if (start.empty())
    {
        return false;
    }

    // display space: undo the per-axis stretch of the shape fit so the shape keeps its proportions
    auto to_display = [&](const point& p) { return point{p.x / options.aspect_x, p.y / options.aspect_y}; };

    std::vector<point> all;
    for (const auto* set : {&start, &final, &targets})
        for (const point& p : *set) all.push_back(to_display(p));

    point min;
    point max;
    bounding_box(all, min, max);
    const double range_x = std::max(max.x - min.x, 1e-12);
    const double range_y = std::max(max.y - min.y, 1e-12);

    // uniform scale: the longer display axis gets panel_size - 2 * margin pixels
    const int inner = options.panel_size - 2 * options.margin;
    const double scale = inner / std::max(range_x, range_y);
    const int panel_w = static_cast<int>(std::ceil(range_x * scale)) + 2 * options.margin;
    const int panel_h = static_cast<int>(std::ceil(range_y * scale)) + 2 * options.margin;
    const int gap = 20;

    canvas img(2 * panel_w + gap, panel_h);

    auto to_screen = [&](const point& p, int panel_x0) {
        const point d = to_display(p);
        return point{panel_x0 + options.margin + (d.x - min.x) * scale,
                     options.margin + (max.y - d.y) * scale}; // flip y for the screen
    };

    const rgb frame{200, 200, 200};
    const rgb line_grey{150, 150, 150};
    const rgb target_grey{215, 215, 215};
    const double radius = std::max(2.0, options.panel_size / 300.0);
    const double n = std::max<double>(1.0, static_cast<double>(start.size() - 1));

    // left panel: start as line chart
    const int left = 0;
    img.rect(left, 0, left + panel_w - 1, panel_h - 1, frame);
    for (size_t i = 1; i < start.size(); i++)
    {
        const point a = to_screen(start[i - 1], left);
        const point b = to_screen(start[i], left);
        img.line(a.x, a.y, b.x, b.y, line_grey, 1.5);
    }
    for (size_t i = 0; i < start.size(); i++)
    {
        const point s = to_screen(start[i], left);
        img.fill_circle(s.x, s.y, radius * 0.6, viridis(i / n));
    }

    // right panel: targets (grey) + final scatter
    const int right = panel_w + gap;
    img.rect(right, 0, right + panel_w - 1, panel_h - 1, frame);
    for (const point& t : targets)
    {
        const point s = to_screen(t, right);
        img.fill_circle(s.x, s.y, radius * 1.4, target_grey);
    }
    for (size_t i = 0; i < final.size(); i++)
    {
        const point s = to_screen(final[i], right);
        img.fill_circle(s.x, s.y, radius, viridis(i / n), 0.9);
    }

    return stbi_write_png(path.c_str(), img.width, img.height, 3, img.data.data(), img.width * 3) != 0;
}
