#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool loadImage(const std::string& IMAGE_PATH, image& image, int mode)
{
    int channels_file = 0;

    unsigned char* data = stbi_load(IMAGE_PATH.c_str(), &image.width, &image.height, &channels_file, mode);
    
    if(!data)
    {
        return false;
    }

    image.mode = mode;

    image.channels = (mode != 0) ? mode : channels_file;

    image.pixels.assign(data, data + image.width * image.height * image.channels);
    stbi_image_free(data);
    return true;
}

unsigned char calculate_otsu_threshold(const image& image) {
    if (image.pixels.empty()) return 0;

    // get frequency of grey values
    std::array<int, 256> histogram = {0};
    for (unsigned char pixel : image.pixels) {
        histogram[pixel]++;
    }

    int total_pixels = image.pixels.size();

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
    for (int t = 0; t < 256; ++t) {
        weight_background += histogram[t]; //pixels in backgroud
        if (weight_background == 0) continue;

        weight_foreground = total_pixels - weight_background; //pixels in the foreground
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
        if (variance_between > max_variance) {
            max_variance = variance_between;
            best_threshold = static_cast<unsigned char>(t);
        }
    }

    return best_threshold;
}

void flatten_alpha_to_white(image& img) {
    if (img.channels != 2) return;
    std::vector<unsigned char> grey(img.width * img.height);
    for (size_t i = 0; i < grey.size(); ++i) {
        unsigned g = img.pixels[2*i], a = img.pixels[2*i + 1];
        grey[i] = (g * a + 255 * (255 - a) + 127) / 255;
    }
    img.pixels = std::move(grey);
    img.channels = 1;
}

void apply_threshold(image& image, unsigned char threshold) {
    for (unsigned char& pixel : image.pixels) {
        pixel = (pixel > threshold) ? 255 : 0;
    }
}

bool saveImage(const std::string& path, const image& img)
{
    if (img.pixels.empty() || img.channels == 0) {
        return false;
    }

    int stride = img.width * img.channels;   // Bytes pro Bildzeile
    int result = stbi_write_png(path.c_str(), img.width, img.height,
                                img.channels, img.pixels.data(), stride);
    return result != 0;   // stbi_write_png gibt 0 bei Fehler zurück
}