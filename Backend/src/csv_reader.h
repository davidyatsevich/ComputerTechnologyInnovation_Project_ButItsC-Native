#ifndef CSV_READER_H
#define CSV_READER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <Eigen/Dense>

using MatrixXd = Eigen::MatrixXd;
using VectorXd = Eigen::VectorXd;
using RowVectorXd = Eigen::RowVectorXd;

struct CSVData
{
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::map<std::string, int> column_index;
};

class CSVReader
{
public:
    static CSVData read_csv_from_file(const std::string &filepath);
    static CSVData read_csv_from_bytes(const std::vector<uint8_t> &bytes);

    static std::string write_csv_to_string(
        const std::vector<std::string> &headers,
        const std::vector<std::vector<std::string>> &rows);

    static std::vector<uint8_t> write_csv_to_bytes(
        const std::vector<std::string> &headers,
        const std::vector<std::vector<std::string>> &rows);

private:
    static CSVData parse_csv_string(const std::string &content);
    static std::string trim(const std::string &str);
    static std::vector<std::string> split_csv_line(const std::string &line);
};

#endif