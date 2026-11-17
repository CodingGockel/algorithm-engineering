#pragma once

#include <string>
#include "csv_data.h"

// Reads a 2-column CSV (header + rows). Columns may contain ISO dates ("YYYY-MM-DD"),
// which are mapped to calendar days since 1970-01-01, or plain numbers.
// Fills data.data, data.x_axis, data.y_axis and data.length (= number of data rows).
// Returns false and sets error on failure.
bool load_series(const std::string& path, csv_data& data, bool& x_is_date, std::string& error,
                 char delim = DEFAULT_DELIM);
