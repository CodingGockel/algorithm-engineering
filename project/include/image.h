#pragma once
#include <string>
#include <vector>
#include<array>
#include<cstdint>

struct pixel
{
    std::uint8_t grey;
    std::uint8_t alpha;

};

struct image
{
    std::vector<pixel> pixels;
    int width = 0;
    int height = 0;
    bool has_alpha = false;
    pixel& at(int x, int y)       
    { 
        return pixels[y * width + x]; 
    }

    const pixel& at(int x, int y) const 
    { 
        return pixels[y * width + x]; 
    }
};

struct point
{
    double x, y;
};

bool load_image(const std::string& IMAGE_PATH, image& image);
uint8_t calculate_otsu_threshold(image& image);
void apply_threshold_mask(image& image, uint8_t threshold);
void apply_alpha_mask(image& image);
bool save_image(const std::string& OUTPUT_PATH, const image& image);
void mask_image(image& imgage);