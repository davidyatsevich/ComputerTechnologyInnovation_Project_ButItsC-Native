
#include <filesystem>
#include <iostream>
#include <string>

#include "pipeline.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    std::cout << "Email Clustering Backend Library" << std::endl;
    std::cout << "=================================" << std::endl;

    // Example usage - this is a library, not a standalone app
    if (argc < 2) {
        std::cout << "\nUsage: email_clustering_backend <command> [options]" << std::endl;
        std::cout << "\nCommands:" << std::endl;
        std::cout << "  kmeans <input.csv> [clusters]  - Process with KMeans clustering"
                  << std::endl;
        std::cout << "  gru <input.csv>                - Process with GRU spam detection"
                  << std::endl;
        std::cout << "\nExample:" << std::endl;
        std::cout << "  email_clustering_backend kmeans data.csv 3" << std::endl;
        return 0;
    }

    std::string command = argv[1];

    if (command == "kmeans" && argc >= 3) {
        std::string input_file = argv[2];
        int clusters = (argc >= 4) ? std::stoi(argv[3]) : 2;

        if (!fs::exists(input_file)) {
            std::cerr << "Error: File not found: " << input_file << std::endl;
            return 1;
        }

        std::cout << "\nProcessing with KMeans (clusters=" << clusters << ")..." << std::endl;
        auto result = Pipeline::process_kmeans(input_file, clusters);

        if (result.success) {
            std::cout << "Success!" << std::endl;
            std::cout << "Summary: " << result.summary.dump(2) << std::endl;

            // Save output CSV
            std::string output_file = "output_kmeans.csv";
            std::cout << Pipeline::save_csv(result.csv_data, output_file) << std::endl;
        } else {
            std::cerr << "Error: " << result.error_message << std::endl;
            return 1;
        }

    } else if (command == "gru" && argc >= 3) {
        std::string input_file = argv[2];

        if (!fs::exists(input_file)) {
            std::cerr << "Error: File not found: " << input_file << std::endl;
            return 1;
        }

        std::cout << "\nProcessing with GRU spam detection..." << std::endl;
        auto result = Pipeline::process_gru(input_file);

        if (result.success) {
            std::cout << "Success!" << std::endl;
            std::cout << "Summary: " << result.summary.dump(2) << std::endl;

            // Save output CSV
            std::string output_file = "output_gru.csv";
            std::cout << Pipeline::save_csv(result.csv_data, output_file) << std::endl;
        } else {
            std::cerr << "Error: " << result.error_message << std::endl;
            return 1;
        }

    } else {
        std::cerr << "Unknown command or invalid arguments" << std::endl;
        return 1;
    }

    return 0;
}
