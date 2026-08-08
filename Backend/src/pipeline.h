#ifndef PIPELINE_H
#define PIPELINE_H

#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct ProcessingResult {
    json summary;
    std::vector<uint8_t> csv_data;
    std::string error_message;
    bool success;
};

class Pipeline {
   public:
    static ProcessingResult process_kmeans(const std::string &file_path, int clusters = 2);
    static ProcessingResult process_gru(const std::string &file_path);
    static std::string save_csv(const std::vector<uint8_t> &csv_data,
                                const std::string &output_path);
};

#endif
