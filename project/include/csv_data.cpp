#include<csv_data.h>

void read_csv_data(const std::string PATH_TO_FILE, csv_data& data_csv, const char delim = ',', const int dimension = 2)
{
    std::ifstream file(PATH_TO_FILE);

    if (!file.is_open()) {
        std::cerr << "cant read csv file at path" << PATH_TO_FILE << std::endl;
        return;
    }

    std::string line;
    int idx = 0;

    while (std::getline(file, line)) {
        std::vector<std::string> line_splitted = split(line, delim);

        if(line_splitted.size() != dimension)
        {
            std::cout << "dimension of file and selected dimension does not match";
        }

        if(idx = 0)
        {
            data_csv.x_axis = line_splitted[0];
            data_csv.y_axis = line_splitted[1];
            continue;
        }
        data_csv.data.push_back({std::stod(line_splitted[0]), std::stod(line_splitted[1])});
        idx++;
    }
    file.close();

    data_csv.length = idx;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while(getline(ss, item, delim)) 
    {
        result.push_back(item);
    }

    return result;
}