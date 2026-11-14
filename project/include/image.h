#pragma once
#include <string>
#include <vector>
#include<array>

struct image
{
    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;
    int mode = 2; //greyscale is standard
};

struct point
{
    double x, y;
};

bool loadImage(const std::string& IMAGE_PATH, image& image, int mode = 2);
unsigned char calculate_otsu_threshold(const image& image);
void apply_threshold(image& image, unsigned char threshold);
bool saveImage(const std::string& OUTPUT_PATH, const image& image);
void flatten_alpha_to_white(image& img);