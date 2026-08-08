#include "pipeline.h"

#include <fstream>
#include <iostream>

#include "csv_reader.h"
#include "gru_processor.h"
#include "kmeans_processor.h"

ProcessingResult Pipeline::process_kmeans(const std::string &file_path, int clusters) {
    ProcessingResult result;
    result.success = false;

    try {
        // Read CSV file
        CSVData data = CSVReader::read_csv_from_file(file_path);

        // Convert to bytes
        auto csv_str = CSVReader::write_csv_to_string(data.headers, data.rows);
        std::vector<uint8_t> csv_bytes(csv_str.begin(), csv_str.end());

        // Process with KMeans
        auto kmeans_result = KMeansProcessor::process_email_csv(csv_bytes, clusters);

        result.summary = kmeans_result.summary;
        result.csv_data = kmeans_result.csv_bytes;
        result.success = true;

    } catch (const std::exception &e) {
        result.error_message = std::string("KMeans processing failed: ") + e.what();
        result.success = false;
    }

    return result;
}

ProcessingResult Pipeline::process_gru(const std::string &file_path) {
    ProcessingResult result;
    result.success = false;

    try {
        // Read CSV file
        CSVData data = CSVReader::read_csv_from_file(file_path);

        // Convert to bytes
        auto csv_str = CSVReader::write_csv_to_string(data.headers, data.rows);
        std::vector<uint8_t> csv_bytes(csv_str.begin(), csv_str.end());

        // Process with GRU
        auto gru_result = GRUProcessor::process_email_csv_gru(csv_bytes);

        result.summary = gru_result.summary;
        result.csv_data = gru_result.csv_bytes;
        result.success = true;

    } catch (const std::exception &e) {
        result.error_message = std::string("GRU processing failed: ") + e.what();
        result.success = false;
    }

    return result;
}

std::string Pipeline::save_csv(const std::vector<uint8_t> &csv_data,
                               const std::string &output_path) {
    try {
        std::ofstream file(output_path, std::ios::binary);
        if (!file.is_open()) {
            return "Failed to open file: " + output_path;
        }

        file.write(reinterpret_cast<const char *>(csv_data.data()), csv_data.size());
        file.close();

        return "CSV saved successfully to: " + output_path;

    } catch (const std::exception &e) {
        return std::string("Failed to save CSV: ") + e.what();
    }
}
