#ifndef KMEANS_PROCESSOR_H
#define KMEANS_PROCESSOR_H

#include <Eigen/Dense>
#include <map>
#include <string>
#include <vector>

#include "csv_reader.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using MatrixXd = Eigen::MatrixXd;
using VectorXd = Eigen::VectorXd;

struct KMeansResult {
    json summary;
    std::vector<uint8_t> csv_bytes;
};

class KMeansProcessor {
   public:
    static KMeansResult process_email_csv(const std::vector<uint8_t> &file_bytes, int clusters = 2,
                                          bool reuse = false);

   private:
    static std::string pick_message_column(const CSVData &data);
    static std::vector<std::string> tokenize(const std::string &text);
    static std::string normalize_text(const std::string &text);
    static MatrixXd compute_tfidf(const std::vector<std::string> &messages);
    static std::vector<int> kmeans_clustering(const MatrixXd &X, int k);
    static MatrixXd compute_pca_2d(const MatrixXd &X, int n_samples = 10000);
};

#endif
