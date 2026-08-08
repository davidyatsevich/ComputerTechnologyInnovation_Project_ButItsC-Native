
#include "gru_processor.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <random>
#include <regex>
#include <set>

static const std::vector<std::string> MSG_CANDIDATES = {"text",  "message", "Message", "email",
                                                        "Email", "body",    "Body",    "v2"};

static const std::vector<std::string> LABEL_CANDIDATES = {
    "spam", "label", "Label", "Category", "category", "type", "Type", "v1", "label_num"};

static const std::vector<std::set<std::string>> LABEL_LIKE_SETS = {
    {"spam", "ham"}, {"0", "1"}, {"true", "false"}, {"yes", "no"}};

std::string GRUProcessor::normalize_text(const std::string &text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    // Remove URLs
    result = std::regex_replace(result, std::regex(R"(https?://\S+|www\.\S+)"), " ");
    // Remove special characters
    result = std::regex_replace(result, std::regex(R"([^a-z0-9\s])"), " ");
    // Collapse whitespace
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");

    // Trim
    auto start = result.begin();
    while (start != result.end() && std::isspace(*start)) start++;
    auto end = result.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

std::vector<std::string> GRUProcessor::tokenize(const std::string &text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;

    while (iss >> token) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string GRUProcessor::pick_message_column(const CSVData &data) {
    // Check for common message column names
    for (const auto &candidate : MSG_CANDIDATES) {
        if (data.column_index.find(candidate) != data.column_index.end()) {
            return candidate;
        }
    }

    // Fallback to first column
    if (!data.headers.empty()) {
        return data.headers[0];
    }

    throw std::runtime_error("No message column found");
}

int GRUProcessor::predict_spam(const std::string &message) {
    // Simplified spam detection heuristic
    std::string normalized = normalize_text(message);

    // Spam indicators
    static const std::vector<std::string> spam_keywords = {
        "free",    "click",  "buy",     "winner",  "prize",  "urgent", "limited",
        "act now", "verify", "confirm", "account", "update", "suspend"};

    int spam_score = 0;
    for (const auto &keyword : spam_keywords) {
        if (normalized.find(keyword) != std::string::npos) {
            spam_score++;
        }
    }

    // Check for excessive capitals
    int capital_count = 0;
    for (char c : message) {
        if (std::isupper(c)) capital_count++;
    }
    if (capital_count > message.length() * 0.3) {
        spam_score++;
    }

    // Check for multiple exclamation marks
    int exclamation_count = std::count(message.begin(), message.end(), '!');
    if (exclamation_count > 2) {
        spam_score++;
    }

    return spam_score > 2 ? 1 : 0;
}

MatrixXd GRUProcessor::compute_tfidf(const std::vector<std::string> &messages) {
    // Build vocabulary
    std::map<std::string, int> vocab;
    std::vector<std::vector<std::string>> tokenized_docs;

    for (const auto &msg : messages) {
        auto tokens = tokenize(normalize_text(msg));
        tokenized_docs.push_back(tokens);
        for (const auto &token : tokens) {
            vocab[token]++;
        }
    }

    // Cap vocabulary size to the most frequent terms. Without this, real-world
    // datasets can produce tens of thousands of unique tokens, which blows up
    // the dense TF-IDF matrix (and everything downstream that depends on its
    // width, like PCA's covariance matrix) to a size that is computationally
    // infeasible.
    constexpr int kMaxVocabSize = 1000;
    std::vector<std::string> features;
    if (static_cast<int>(vocab.size()) > kMaxVocabSize) {
        std::vector<std::pair<std::string, int>> vocab_by_freq(vocab.begin(), vocab.end());
        std::partial_sort(vocab_by_freq.begin(), vocab_by_freq.begin() + kMaxVocabSize,
                          vocab_by_freq.end(),
                          [](const auto &a, const auto &b) { return a.second > b.second; });
        vocab_by_freq.resize(kMaxVocabSize);
        for (const auto &[word, _] : vocab_by_freq) {
            features.push_back(word);
        }
        std::sort(features.begin(), features.end());
    } else {
        for (const auto &[word, _] : vocab) {
            features.push_back(word);
        }
    }
    std::set<std::string> feature_set(features.begin(), features.end());

    // Build TF-IDF matrix
    int n_docs = messages.size();
    int n_features = features.size();
    MatrixXd tfidf_matrix = MatrixXd::Zero(n_docs, n_features);

    std::map<std::string, int> feature_index;
    for (int i = 0; i < n_features; ++i) {
        feature_index[features[i]] = i;
    }

    // Calculate TF-IDF (only over terms actually present in each doc, rather
    // than looping over the full vocabulary for every document)
    for (int doc_idx = 0; doc_idx < n_docs; ++doc_idx) {
        std::map<std::string, int> tf;
        for (const auto &term : tokenized_docs[doc_idx]) {
            if (feature_set.count(term)) {
                tf[term]++;
            }
        }

        int doc_len = tokenized_docs[doc_idx].size();
        if (doc_len == 0) continue;

        for (const auto &[feature, count] : tf) {
            double term_freq = static_cast<double>(count) / doc_len;
            double doc_freq = static_cast<double>(vocab[feature]) / n_docs;
            double idf = std::log(1.0 / doc_freq);
            tfidf_matrix(doc_idx, feature_index[feature]) = term_freq * idf;
        }
    }

    return tfidf_matrix;
}

MatrixXd GRUProcessor::compute_pca_2d(const MatrixXd &X) {
    int n_samples = X.rows();
    int n_features = X.cols();

    // Center the data
    VectorXd mean = X.colwise().mean();
    MatrixXd X_centered = X.rowwise() - mean.transpose();

    MatrixXd eigenvectors;

    // When there are far more features than samples (common for TF-IDF data),
    // computing the n_features x n_features covariance matrix directly is
    // prohibitively expensive. Instead, decompose the much smaller
    // n_samples x n_samples Gram matrix and map the eigenvectors back into
    // feature space.
    if (n_features > n_samples) {
        MatrixXd gram = (X_centered * X_centered.transpose()) / (n_samples - 1);
        Eigen::SelfAdjointEigenSolver<MatrixXd> solver(gram);
        MatrixXd small_eigenvectors = solver.eigenvectors();

        MatrixXd top2 = small_eigenvectors.block(
            0, std::max(0, static_cast<int>(small_eigenvectors.cols()) - 2),
            small_eigenvectors.rows(), 2);
        eigenvectors = X_centered.transpose() * top2;
        for (int c = 0; c < eigenvectors.cols(); ++c) {
            double norm = eigenvectors.col(c).norm();
            if (norm > 1e-10) eigenvectors.col(c) /= norm;
        }
    } else {
        MatrixXd cov = (X_centered.transpose() * X_centered) / (n_samples - 1);
        Eigen::SelfAdjointEigenSolver<MatrixXd> solver(cov);
        MatrixXd all_eigenvectors = solver.eigenvectors();
        eigenvectors = all_eigenvectors.block(
            0, std::max(0, static_cast<int>(all_eigenvectors.cols()) - 2),
            all_eigenvectors.rows(), 2);
    }

    // Project data
    return X_centered * eigenvectors;
}

GRUResult GRUProcessor::process_email_csv_gru(const std::vector<uint8_t> &file_bytes) {
    CSVData data = CSVReader::read_csv_from_bytes(file_bytes);

    // Pick message column
    std::string msg_col = pick_message_column(data);
    int msg_col_idx = data.column_index[msg_col];

    std::vector<std::string> messages;
    for (const auto &row : data.rows) {
        if (msg_col_idx < row.size()) {
            messages.push_back(row[msg_col_idx]);
        }
    }

    if (messages.empty()) {
        throw std::runtime_error("CSV file has no data rows to process.");
    }

    // Predict spam for each message
    std::vector<int> predictions;
    std::map<int, int> counts;
    std::map<int, std::vector<std::string>> samples;

    for (const auto &msg : messages) {
        int pred = predict_spam(msg);
        predictions.push_back(pred);
        counts[pred]++;

        if (samples[pred].size() < 10) {
            samples[pred].push_back(msg);
        }
    }

    // Compute TF-IDF for visualization
    MatrixXd X = compute_tfidf(messages);
    MatrixXd X_pca = compute_pca_2d(X);

    // Build JSON response
    json summary;
    summary["total"] = static_cast<int>(messages.size());
    summary["clusters"] = 2;

    // Samples
    for (int c = 0; c < 2; ++c) {
        if (samples.find(c) != samples.end()) {
            for (const auto &msg : samples[c]) {
                summary["samples"][std::to_string(c)].push_back(msg);
            }
        }
    }

    // Counts
    for (const auto &[cluster, count] : counts) {
        summary["counts"][std::to_string(cluster)] = count;
    }

    // PCA points
    json pca_points = json::array();
    for (int i = 0; i < std::min(10000, static_cast<int>(X_pca.rows())); ++i) {
        json point;
        point["x"] = X_pca(i, 0);
        point["y"] = X_pca(i, 1);
        point["cluster"] = predictions[i];
        pca_points.push_back(point);
    }
    summary["pca"]["points"] = pca_points;

    // Sunburst
    json sunburst;
    sunburst["name"] = "root";
    sunburst["children"] = json::array();

    for (int c = 0; c < 2; ++c) {
        json cluster_node;
        cluster_node["name"] = (c == 0) ? "Class 0 (Ham)" : "Class 1 (Spam)";
        cluster_node["children"] = json::array();
        sunburst["children"].push_back(cluster_node);
    }
    summary["sunburst"] = sunburst;

    // Build output CSV
    std::vector<std::string> headers = {"email", "clusterid"};
    std::vector<std::vector<std::string>> rows;

    for (int i = 0; i < messages.size(); ++i) {
        rows.push_back({messages[i], std::to_string(predictions[i])});
    }

    auto csv_bytes = CSVReader::write_csv_to_bytes(headers, rows);

    GRUResult result;
    result.summary = summary;
    result.csv_bytes = csv_bytes;

    return result;
}
