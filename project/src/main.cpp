#include <iostream>
#include "constants.h"
#include <string>
#include <vector>
#include "image.h"
#include "csv_data.h"

int main()
{
    image image;
    csv_data c_data;
    load_image("resources/images/ok.png", image);
    mask_image(image);
    image.points.reserve(image.height * image.width);
    get_outline(image, image.points);
    save_image("build/images/out.png", image);
    /*
    for (const auto& p : image.points) 
    {
        std::cout << "(" << p.x << "/" << p.y << ")" << ",";
    }
    */
    read_csv_data("resources/data/aapl_2015_2017.csv", c_data);

    for (const point& p : c_data.data)
    {
        std::cout << "(" << p.x << "/" << p.y << ")" << std::endl;
    }

    return 0;
}



