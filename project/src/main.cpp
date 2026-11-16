#include <iostream>
#include "constants.h"
#include <string>
#include <vector>
#include "image.h"

int main()
{
    image image;

    load_image("resources/images/ok.png", image);

    std::vector<point> points;
    points.reserve(image.height * image.width);

    mask_image(image);

    get_outline(image, points);

    save_image("build/images/out.png", image);

    for (const auto& p : points) 
    {
        std::cout << "(" << p.x << "/" << p.y << ")" << ",";
    }

    return 0;
}



