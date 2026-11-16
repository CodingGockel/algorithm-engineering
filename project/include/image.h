#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <point.h>

#define GREYSCALE_ALPHA_MODE 2

#define MAX_ALPHA_VALUE 255
#define MIN_ALPHA_VALUE 0
#define THRESHOLD_ALPHA_VALUE 128

#define MAX_GREY_VALUE 255
#define MIN_GREY_VALUE 0

struct pixel
{
    std::uint8_t grey;
    std::uint8_t alpha;
};

struct image
{
    std::vector<pixel> pixels;
    std::vector<point> points;
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

    bool has_white_neighbor(int x, int y)
    {
        pixel& p = at(x, y);
        if(p.grey == MAX_GREY_VALUE)
        {
            return false;
        }
        
        if(x == 0 || x == width - 1 || y == 0 || y == height - 1)
        {
            return true;
        }

        if(at(x-1, y-1).grey != MAX_GREY_VALUE)
        {
            if(at(x, y-1).grey != MAX_GREY_VALUE)
            {
                if(at(x+1, y-1).grey != MAX_GREY_VALUE)
                {
                    if(at(x-1, y).grey != MAX_GREY_VALUE)
                    {
                        if(at(x-1, y+1).grey != MAX_GREY_VALUE)
                        {
                            if(at(x, y+1).grey != MAX_GREY_VALUE)
                            {
                                if(at(x+1, y+1).grey != MAX_GREY_VALUE)
                                {
                                    if(at(x+1, y).grey != MAX_GREY_VALUE)
                                    {
                                        return false;
                                    }
                                }
                            }
                        }
                    }   
                }
            }
        }
        return true;
    }
};

bool load_image(const std::string& IMAGE_PATH, image& image);
bool save_image(const std::string& OUTPUT_PATH, const image& image);

void mask_image(image& imgage);
void apply_alpha_mask(image& image);
void apply_threshold_mask(image& image, uint8_t threshold);
uint8_t calculate_otsu_threshold(image& image);
void get_outline(image& image, std::vector<point>& points);