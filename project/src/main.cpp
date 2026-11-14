#include <iostream>
#include "constants.h"
#include <string>
#include <vector>
#include "image.h"

int main()
{
    image image;
    char threshold;


    loadImage("resources/images/baer.png", image);
    
    flatten_alpha_to_white(image);

    threshold = calculate_otsu_threshold(image);

    apply_threshold(image, threshold);

    saveImage("build/images/out.png", image);

    return 0;
}



