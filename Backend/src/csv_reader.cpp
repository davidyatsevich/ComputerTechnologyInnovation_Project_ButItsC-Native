#include "csv_reader.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

std::string CSVReader::trim(const std::string &str)
{
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start))
    {
        start++;
    }

    auto end = str.end();
    do
    {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

std::vector<std::string> CSVReader::split_csv_line(const std::string &line)
{
    std::vector<std::string> result;
    std::string current;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];

        if (c == '"')
        {
            in_quotes = !in_quotes;
            current += c;
        }
        else if (c == ',' && !in_quotes)
        {
            result.push_back(trim(current));
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    result.push_back(trim(current));
    return result;
}

CSVData CSVReader::parse_csv_string(const std::string &content)
{
    CSVData data;
    std::istringstream stream(content);
    std::string line;
    bool first_line = true;

    while (std::getline(stream, line))
    {
        if (line.empty())
            continue;

        auto fields = split_csv_line(line);

        if (first_line)
        {
            data.headers = fields;

            for (size_t i = 0; i < fields.size(); ++i)
            {
                data.column_index[fields[i]] = i;
            }

            first_line = false;
        }
        else
        {
            data.rows.push_back(fields);
        }
    }

    return data;
}

CSVData CSVReader::read_csv_from_file(const std::string &filepath)
{
    std::ifstream file(filepath);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return parse_csv_string(buffer.str());
}

CSVData CSVReader::read_csv_from_bytes(const std::vector<uint8_t> &bytes)
{
    std::string content(bytes.begin(), bytes.end());
    return parse_csv_string(content);
}

std::string CSVReader::write_csv_to_string(
    const std::vector<std::string> &headers,
    const std::vector<std::vector<std::string>> &rows)
{
    std::ostringstream oss;

    for (size_t i = 0; i < headers.size(); ++i)
    {
        oss << headers[i];

        if (i < headers.size() - 1)
            oss << ",";
    }

    oss << "\n";

    for (const auto &row : rows)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            oss << row[i];

            if (i < row.size() - 1)
                oss << ",";
        }

        oss << "\n";
    }

    return oss.str();
}

std::vector<uint8_t> CSVReader::write_csv_to_bytes(
    const std::vector<std::string> &headers,
    const std::vector<std::vector<std::string>> &rows)
{
    std::string csv_str = write_csv_to_string(headers, rows);
    return std::vector<uint8_t>(csv_str.begin(), csv_str.end());
}