#pragma once

#include <vector>
#include <string>
#include <point.h>
#include <fstream>
#include <sstream>
#include <iostream>

#define DEFAULT_DELIM ','

struct csv_data
{
    std::vector<point> data;
    std::string x_axis;
    std::string y_axis;
    int length = 0;
};

void read_csv_data(const std::string PATH_TO_FILE, csv_data& data, const char delim = ',', const size_t dimension = 2);
std::vector<std::string> split(const std::string &s, char delim);


