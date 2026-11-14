#include <iostream>
#include "constants.h"
#include <string>
#include <vector>
#include "image.h"

int main()
{
    image image;

    load_image("resources/images/baer.png", image);

    mask_image(image);

    save_image("build/images/out.png", image);

    return 0;
}



