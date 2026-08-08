#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <crow.h> // Using Crow as a popular C++ micro-framework similar to FastAPI

// Note: A direct 1:1 translation of FastAPI (Python) to C++ requires a web framework
// like Crow, Drogon, or Pistache. Below is an equivalent implementation using the Crow framework.

std::vector<char> LATEST_PROCESSED_CSV;
std::vector<char> LATEST_PROCESSED_CSV_GRU;

// Dummy pipeline functions (to be implemented or linked from C++ ML libraries)
std::pair<std::string, std::vector<char>> process_email_csv(const std::vector<char> &file_bytes, int clusters, bool reuse)
{
    // Implement or call C++ ML pipeline here
    return {"{\"status\": \"success\"}", {'c', 's', 'v', '_', 'd', 'a', 't', 'a'}};
}

std::pair<std::string, std::vector<char>> process_email_csv_gru(const std::vector<char> &file_bytes)
{
    // Implement or call C++ GRU pipeline here
    return {"{\"status\": \"success_gru\"}", {'g', 'r', 'u', '_', 'd', 'a', 't', 'a'}};
}

int main()
{
    crow::SimpleApp app;

    // CORS middleware configuration in Crow can be handled via headers or custom middleware
    // For simplicity, we add an after_handler for CORS headers
    app.global_ordinary_handlers().push_after_handler([](crow::response &res)
                                                      {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type"); });

    // Health check route
    CROW_ROUTE(app, "/health").methods("GET"_method)([]()
                                                     { return crow::json::wvalue{{"status", "ok"}}; });

    // Process CSV route
    CROW_ROUTE(app, "/process_csv").methods("POST"_method)([](const crow::request &req)
                                                           {
        try {
            int clusters = 2;
            bool reuse_clusters = false;

            // Parse query parameters
            auto query_string = req.url_params;
            if (query_string.get("clusters")) {
                clusters = std::stoi(query_string.get("clusters"));
            }
            if (query_string.get("reuse_clusters")) {
                std::string reuse = query_string.get("reuse_clusters");
                reuse_clusters = (reuse == "true" || reuse == "1");
            }

            if (clusters < 2 || clusters > 12) {
                return crow::response(400, "Processing failed: Clusters must be between 2 and 12");
            }

            // Extract file bytes from request body
            std::vector<char> file_bytes(req.body.begin(), req.body.end());

            auto [summary, csv_bytes] = process_email_csv(file_bytes, clusters, reuse_clusters);
            LATEST_PROCESSED_CSV = csv_bytes;

            return crow::response(200, summary);
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Processing failed: ") + e.what());
        } });

    // Download CSV route
    CROW_ROUTE(app, "/download_csv").methods("GET"_method)([]()
                                                           {
        if (LATEST_PROCESSED_CSV.empty()) {
            return crow::response(404, "No processed CSV available yet.");
        }

        crow::response res;
        res.code = 200;
        res.set_header("Content-Type", "text/csv");
        res.set_header("Content-Disposition", "attachment; filename=cleaned_emails.csv");
        res.write(std::string(LATEST_PROCESSED_CSV.begin(), LATEST_PROCESSED_CSV.end()));
        return res; });

    // Process CSV GRU route
    CROW_ROUTE(app, "/process_csv_gru").methods("POST"_method)([](const crow::request &req)
                                                               {
        try {
            std::vector<char> file_bytes(req.body.begin(), req.body.end());
            auto [summary, csv_bytes] = process_email_csv_gru(file_bytes);
            LATEST_PROCESSED_CSV_GRU = csv_bytes;
            return crow::response(200, summary);
        } catch (const std::exception& e) {
            return crow::response(400, std::string("GRU processing failed: ") + e.what());
        } });

    // Download CSV GRU route
    CROW_ROUTE(app, "/download_csv_gru").methods("GET"_method)([]()
                                                               {
        if (LATEST_PROCESSED_CSV_GRU.empty()) {
            return crow::response(404, "No GRU-processed CSV available yet.");
        }

        crow::response res;
        res.code = 200;
        res.set_header("Content-Type", "text/csv");
        res.set_header("Content-Disposition", "attachment; filename=gru_spam_sorted.csv");
        res.write(std::string(LATEST_PROCESSED_CSV_GRU.begin(), LATEST_PROCESSED_CSV_GRU.end()));
        return res; });

    // Run the app on port 8000
    app.port(8000).multithreaded().run();
    return 0;
}