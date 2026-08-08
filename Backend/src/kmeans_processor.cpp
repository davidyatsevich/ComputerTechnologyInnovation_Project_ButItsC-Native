
#include "kmeans_processor.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <set>

static const std::vector<std::string> MSG_CANDIDATES = {"message", "text",  "body", "sms",
                                                        "Email",   "email", "v2"};

static const std::vector<std::set<std::string>> LABEL_LIKE_SETS = {
    {"spam", "ham"}, {"0", "1"}, {"true", "false"}, {"yes", "no"}};

std::string KMeansProcessor::normalize_text(const std::string &text) {
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

std::vector<std::string> KMeansProcessor::tokenize(const std::string &text) {
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

std::string KMeansProcessor::pick_message_column(const CSVData &data) {
    if (data.headers.empty()) {
        throw std::runtime_error(
            "CSV file has no columns. Check that the file is a valid, non-empty CSV.");
    }

    // Check for common message column names
    for (const auto &candidate : MSG_CANDIDATES) {
        if (data.column_index.find(candidate) != data.column_index.end()) {
            return candidate;
        }
    }

    // For 2-column files, check if one looks like a label
    if (data.headers.size() == 2) {
        auto is_label = [](const std::vector<std::string> &col_values) {
            std::set<std::string> vals;
            for (const auto &v : col_values) {
                std::string lower = v;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                vals.insert(lower);
            }

            for (const auto &label_set : LABEL_LIKE_SETS) {
                if (vals.size() <= label_set.size()) {
                    bool matches = true;
                    for (const auto &v : vals) {
                        if (label_set.find(v) == label_set.end()) {
                            matches = false;
                            break;
                        }
                    }
                    if (matches) return true;
                }
            }
            return false;
        };

        std::vector<std::string> col0_vals, col1_vals;
        for (const auto &row : data.rows) {
            if (row.size() >= 1) col0_vals.push_back(row[0]);
            if (row.size() >= 2) col1_vals.push_back(row[1]);
        }

        if (is_label(col0_vals)) return data.headers[1];
        if (is_label(col1_vals)) return data.headers[0];
    }

    // Fallback: return the longest column by average length
    if (data.rows.empty()) {
        throw std::runtime_error("CSV file has headers but no data rows.");
    }

    int best_idx = 0;
    double max_avg_len = 0;

    for (size_t col_idx = 0; col_idx < data.headers.size(); ++col_idx) {
        double total_len = 0;
        for (const auto &row : data.rows) {
            if (col_idx < row.size()) {
                total_len += row[col_idx].length();
            }
        }
        double avg_len = total_len / data.rows.size();
        if (avg_len > max_avg_len) {
            max_avg_len = avg_len;
            best_idx = col_idx;
        }
    }

    return data.headers[best_idx];
}

MatrixXd KMeansProcessor::compute_tfidf(const std::vector<std::string> &messages) {
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
    // width, like PCA's covariance matrix and k-means' distance calculations)
    // to a size that is computationally infeasible.
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

    // Calculate TF-IDF
    for (int doc_idx = 0; doc_idx < n_docs; ++doc_idx) {
        // Term frequency
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

std::vector<int> KMeansProcessor::kmeans_clustering(const MatrixXd &X, int k) {
    int n_samples = X.rows();
    int n_features = X.cols();

    std::vector<int> labels(n_samples);
    MatrixXd centers = MatrixXd::Zero(k, n_features);

    // Initialize centers randomly
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, n_samples - 1);

    std::set<int> selected;
    for (int i = 0; i < k; ++i) {
        int idx;
        do {
            idx = dis(gen);
        } while (selected.count(idx));
        selected.insert(idx);
        centers.row(i) = X.row(idx);
    }

    // Run K-means iterations
    int max_iterations = 100;
    double tolerance = 1e-4;

    for (int iter = 0; iter < max_iterations; ++iter) {
        MatrixXd old_centers = centers;

        // Assign points to nearest center
        for (int i = 0; i < n_samples; ++i) {
            double min_dist = std::numeric_limits<double>::max();
            int best_cluster = 0;

            for (int c = 0; c < k; ++c) {
                double dist = (X.row(i) - centers.row(c)).norm();
                if (dist < min_dist) {
                    min_dist = dist;
                    best_cluster = c;
                }
            }
            labels[i] = best_cluster;
        }

        // Update centers
        centers = MatrixXd::Zero(k, n_features);
        std::vector<int> counts(k, 0);

        for (int i = 0; i < n_samples; ++i) {
            centers.row(labels[i]) += X.row(i);
            counts[labels[i]]++;
        }

        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                centers.row(c) /= counts[c];
            }
        }

        // Check convergence
        double change = (centers - old_centers).norm();
        if (change < tolerance) break;
    }

    return labels;
}

MatrixXd KMeansProcessor::compute_pca_2d(const MatrixXd &X, int n_samples) {
    int actual_samples = std::min(n_samples, static_cast<int>(X.rows()));
    MatrixXd X_sample = X.block(0, 0, actual_samples, X.cols());

    // Center the data
    VectorXd mean = X_sample.colwise().mean();
    MatrixXd X_centered = X_sample.rowwise() - mean.transpose();

    int n_feat = X_centered.cols();

    MatrixXd eigenvectors;

    // When there are far more features than samples (common for TF-IDF data),
    // computing the n_feat x n_feat covariance matrix directly is prohibitively
    // expensive (both in memory and in the O(n_feat^3) eigendecomposition).
    // Instead, decompose the much smaller n_samples x n_samples Gram matrix and
    // map the eigenvectors back into feature space.
    if (n_feat > actual_samples) {
        MatrixXd gram = (X_centered * X_centered.transpose()) / (actual_samples - 1);
        Eigen::SelfAdjointEigenSolver<MatrixXd> solver(gram);
        MatrixXd small_eigenvectors = solver.eigenvectors();

        // Take the last 2 eigenvectors (largest eigenvalues) and map back to
        // feature space: v_feature = X^T * v_sample (unnormalized, but fine
        // since we only need this for 2D visualization).
        MatrixXd top2 = small_eigenvectors.block(0, small_eigenvectors.cols() - 2,
                                                  small_eigenvectors.rows(), 2);
        eigenvectors = X_centered.transpose() * top2;
        for (int c = 0; c < eigenvectors.cols(); ++c) {
            double norm = eigenvectors.col(c).norm();
            if (norm > 1e-10) eigenvectors.col(c) /= norm;
        }
    } else {
        MatrixXd cov = (X_centered.transpose() * X_centered) / (actual_samples - 1);
        Eigen::SelfAdjointEigenSolver<MatrixXd> solver(cov);
        MatrixXd all_eigenvectors = solver.eigenvectors();
        eigenvectors = all_eigenvectors.block(0, all_eigenvectors.cols() - 2,
                                              all_eigenvectors.rows(), 2);
    }

    // Project data
    return X_centered * eigenvectors;
}

KMeansResult KMeansProcessor::process_email_csv(const std::vector<uint8_t> &file_bytes,
                                                int clusters, bool reuse) {
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

    // Compute TF-IDF
    MatrixXd X = compute_tfidf(messages);

    // K-means clustering
    std::vector<int> labels = kmeans_clustering(X, clusters);

    // Compute PCA for visualization
    MatrixXd X_pca = compute_pca_2d(X);

    // Build samples and counts
    std::map<int, std::vector<std::string>> samples;
    std::map<int, int> counts;

    for (int i = 0; i < labels.size(); ++i) {
        int cluster = labels[i];
        counts[cluster]++;
        if (samples[cluster].size() < 10) {
            samples[cluster].push_back(messages[i]);
        }
    }

    // Build JSON response
    json summary;
    summary["total"] = static_cast<int>(messages.size());
    summary["clusters"] = clusters;

    // Samples
    for (const auto &[cluster, msgs] : samples) {
        for (const auto &msg : msgs) {
            summary["samples"][std::to_string(cluster)].push_back(msg);
        }
    }

    // Counts
    for (const auto &[cluster, count] : counts) {
        summary["counts"][std::to_string(cluster)] = count;
    }

    // PCA points
    json pca_points = json::array();
    int n_plot = std::min(10000, static_cast<int>(X_pca.rows()));
    for (int i = 0; i < n_plot; ++i) {
        json point;
        point["x"] = X_pca(i, 0);
        point["y"] = X_pca(i, 1);
        point["cluster"] = labels[i];
        pca_points.push_back(point);
    }
    summary["pca"]["points"] = pca_points;

    // Sunburst (simplified)
    json sunburst;
    sunburst["name"] = "root";
    sunburst["children"] = json::array();

    for (int c = 0; c < clusters; ++c) {
        json cluster_node;
        cluster_node["name"] = "Cluster " + std::to_string(c);
        cluster_node["children"] = json::array();
        sunburst["children"].push_back(cluster_node);
    }
    summary["sunburst"] = sunburst;

    // Build output CSV
    std::vector<std::string> headers = {"email", "clusterid"};
    std::vector<std::vector<std::string>> rows;

    for (int i = 0; i < messages.size(); ++i) {
        rows.push_back({messages[i], std::to_string(labels[i])});
    }

    auto csv_bytes = CSVReader::write_csv_to_bytes(headers, rows);

    KMeansResult result;
    result.summary = summary;
    result.csv_bytes = csv_bytes;

    return result;
}
