
#ifndef GRU_PROCESSOR_H
#define GRU_PROCESSOR_H

#include <Eigen/Dense>
#include <map>
#include <string>
#include <vector>

#include "csv_reader.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using MatrixXd = Eigen::MatrixXd;
using VectorXd = Eigen::VectorXd;

struct GRUResult {
    json summary;
    std::vector<uint8_t> csv_bytes;
};

class GRUProcessor {
   public:
    static GRUResult process_email_csv_gru(const std::vector<uint8_t> &file_bytes);

   private:
    static std::string pick_message_column(const CSVData &data);
    static std::string normalize_text(const std::string &text);
    static std::vector<std::string> tokenize(const std::string &text);
    static int predict_spam(const std::string &message);
    static MatrixXd compute_tfidf(const std::vector<std::string> &messages);
    static MatrixXd compute_pca_2d(const MatrixXd &X);
};

#endif
