#include "image.h"
#include <cstring>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool load_image(const std::string& IMAGE_PATH, image& image)
{
    int w, h, c;
    unsigned char* data = stbi_load(IMAGE_PATH.c_str(), &w, &h, &c, GREYSCALE_ALPHA_MODE);
    
    if(!data)
    {
        return false;
    }
    
    image.height = h;
    image.width = w;

    const size_t n = (image.width) * image.height;
    image.pixels.resize(n);
    std::memcpy(image.pixels.data(), data, n * sizeof(pixel));
    
    for(pixel& p : image.pixels)
    {
        if(p.alpha < MAX_ALPHA_VALUE)
        {
            image.has_alpha = true;
            break;
        }
    }

    stbi_image_free(data);
    return true;
}

bool save_image(const std::string& path, const image& img)
{
    if (img.pixels.empty()) 
    {
        return false;
    }

    int stride = img.width * 2;
    int result = stbi_write_png(path.c_str(), img.width, img.height, 2, img.pixels.data(), stride);
    return result != 0;
}

void mask_image(image& image)
{
    if(image.has_alpha)
    {
        apply_alpha_mask(image);
    }else{
        uint8_t threshold = calculate_otsu_threshold(image);
        apply_threshold_mask(image, threshold);
    }
}

void apply_alpha_mask(image& image)
{
    for(pixel& p : image.pixels)
    {
        if(p.alpha < THRESHOLD_ALPHA_VALUE)
        {
            p.grey = MAX_GREY_VALUE;
        }else{
            p.grey = MIN_GREY_VALUE;
        }
        p.alpha = MAX_ALPHA_VALUE;
    }
}

void apply_threshold_mask(image& image, uint8_t threshold) {
    for(pixel& p : image.pixels) 
    {
        if(p.grey > threshold)
        {
            p.grey = MAX_GREY_VALUE;
            
        }else{
            p.grey = MIN_GREY_VALUE;
        }
    }
}

std::vector<point> get_outline_points(image& image)
{
    /*
        iterate pixels:
            if(alpha < Threshold alpha)
    */
}

void get_outline(image& image, std::vector<point>& points)
{
    for(int y = 0; y < image.height; y++)
    {
        for(int x = 0; x < image.width; x++)
        {
            if(!image.has_white_neighbor(x,y))
            {
                image.at(x,y).alpha = MIN_ALPHA_VALUE;
            }else{
                points.push_back({double(x), double(y)});
                image.at(x,y).grey = MIN_GREY_VALUE;
            }
        }
    }
}

uint8_t calculate_otsu_threshold(image& image) {
    if (image.pixels.empty()) return 0;

    // get frequency of grey values
    std::array<int, 256> histogram = {0};

    for (pixel& p : image.pixels)
    {
        histogram[p.grey]++;
    }

    int total_pixels_count = static_cast<int>(image.pixels.size());

    //get sum of all grey values
    double sum_total = 0.0;
    for (int i = 0; i < 256; ++i) {
        sum_total += i * histogram[i];
    }

    double sum_background = 0.0;
    int weight_background = 0;
    int weight_foreground = 0;

    double max_variance = 0.0;
    unsigned char best_threshold = 0;

    //calc all possible threshholds
    for (int t = 0; t < 256; ++t) 
    {
        weight_background += histogram[t]; //pixels in backgroud
        if (weight_background == 0) continue;

        weight_foreground = total_pixels_count - weight_background; //pixels in the foreground
        if (weight_foreground == 0) break;

        sum_background += static_cast<double>(t * histogram[t]);

        //mean of the brightness
        double mean_background = sum_background / weight_background;
        double mean_foreground = (sum_total - sum_background) / weight_foreground;

        // variance
        double variance_between = static_cast<double>(weight_background) * 
                                  static_cast<double>(weight_foreground) * 
                                  (mean_background - mean_foreground) * 
                                  (mean_background - mean_foreground);

        // chekc for new max
        if (variance_between > max_variance) 
        {
            max_variance = variance_between;
            best_threshold = static_cast<unsigned char>(t);
        }
    }

    return best_threshold;
}