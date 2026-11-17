#include "date_parser.h"

#include <cctype>
#include <cstdio>
#include <stdexcept>

long days_from_civil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    const long era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);                 // [0, 399]
    const unsigned doy = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1; // [0, 365]
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;                     // [0, 146096]
    return era * 146097 + static_cast<long>(doe) - 719468;
}

std::string trim(const std::string& s)
{
    size_t begin = 0;
    size_t end = s.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) begin++;
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(begin, end - begin);
}

bool parse_iso_date(const std::string& raw, double& out_days)
{
    const std::string s = trim(raw);
    int year = 0;
    unsigned month = 0;
    unsigned day = 0;
    int consumed = 0;

    if (std::sscanf(s.c_str(), "%d-%u-%u%n", &year, &month, &day, &consumed) != 3)
    {
        return false;
    }
    if (static_cast<size_t>(consumed) != s.size())
    {
        return false; // trailing garbage, e.g. a time component we do not support
    }
    if (month < 1 || month > 12 || day < 1 || day > 31)
    {
        return false;
    }

    out_days = static_cast<double>(days_from_civil(year, month, day));
    return true;
}

double parse_value(const std::string& raw, bool& is_date)
{
    double days = 0.0;
    if (parse_iso_date(raw, days))
    {
        is_date = true;
        return days;
    }

    is_date = false;
    const std::string s = trim(raw);
    size_t consumed = 0;
    const double value = std::stod(s, &consumed); // throws std::invalid_argument on failure
    if (consumed != s.size())
    {
        throw std::invalid_argument("not a number or ISO date: '" + s + "'");
    }
    return value;
}
