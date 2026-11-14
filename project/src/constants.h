#pragma once
#include <string_view>

namespace config 
{
    inline constexpr std::string_view RESOURCE_FOLDER_PATH = "resources/";
    inline constexpr std::string_view IMAGE_FOLDER_PATH    = "resources/images/";
    inline constexpr std::string_view DATA_FOLDER_PATH     = "resources/data/";
    inline constexpr std::string_view DEFAULT_IMAGE_FILE   = "rocket.png";
    inline constexpr std::string_view DEFAULT_OUTPUT_PATH   = "resources/output/out.png";
    inline constexpr std::string_view DEFAULT_INPUT_IMAGE_PATH   = "resources/images/rocket.png";
    inline constexpr std::string_view DEFAULT_DATA_FILE    = "data.csv";
    inline constexpr int              TEST = 4;

}