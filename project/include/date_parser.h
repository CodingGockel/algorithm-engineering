#pragma once

#include <string>

// Days since 1970-01-01 in the proleptic Gregorian calendar (Howard Hinnant's algorithm).
// Works for dates before 1970 (negative result).
long days_from_civil(int year, unsigned month, unsigned day);

// Parses "YYYY-MM-DD" into days since epoch. Returns false if the string is not an ISO date.
bool parse_iso_date(const std::string& s, double& out_days);

// Parses a CSV cell: ISO date -> days since epoch, otherwise a plain number.
// Sets is_date accordingly. Throws std::invalid_argument if neither works.
double parse_value(const std::string& s, bool& is_date);

// Removes surrounding whitespace (including '\r' from Windows line endings).
std::string trim(const std::string& s);
