#include "data_loader.h"
#include "date_parser.h"

#include <fstream>
#include <stdexcept>

bool load_series(const std::string& path, csv_data& data, bool& x_is_date, std::string& error, char delim)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        error = "cannot open csv file: " + path;
        return false;
    }

    data = csv_data{};
    x_is_date = false;

    std::string line;
    size_t line_no = 0;
    bool header_read = false;

    while (std::getline(file, line))
    {
        line_no++;
        if (trim(line).empty())
        {
            continue;
        }

        const std::vector<std::string> cells = split(line, delim);
        if (cells.size() != 2)
        {
            error = "line " + std::to_string(line_no) + ": expected 2 columns, got " + std::to_string(cells.size());
            return false;
        }

        if (!header_read)
        {
            data.x_axis = trim(cells[0]);
            data.y_axis = trim(cells[1]);
            header_read = true;
            continue;
        }

        try
        {
            bool x_date = false;
            bool y_date = false;
            const double x = parse_value(cells[0], x_date);
            const double y = parse_value(cells[1], y_date);
            if (data.data.empty())
            {
                x_is_date = x_date;
            }
            data.data.push_back({x, y});
        }
        catch (const std::exception& e)
        {
            error = "line " + std::to_string(line_no) + ": " + e.what();
            return false;
        }
    }

    if (!header_read)
    {
        error = "csv file is empty: " + path;
        return false;
    }

    data.length = static_cast<int>(data.data.size());
    return true;
}
