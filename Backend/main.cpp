#include <fastapi/fastapi.h>
#include <fastapi/middleware/cors.h>
#include <fastapi/responses/json_response.h>
#include <fastapi/responses/streaming_response.h>
#include <fastapi/query.h>
#include "pipeline.h"
#include <iostream>
#include <memory>

using namespace fastapi;

FastAPI app("Email Clustering Backend");

// Permits access for cross-origin frontend access
app.add_middleware<CORSMiddleware>(
    {"*"}, // NOTE: Replace with frontend URL when possible
    true,
    {"*"},
    {"*"});

// Storage for the latest processed file
std::shared_ptr<std::string> LATEST_PROCESSED_CSV = nullptr;
// Buffer for the latest processed CSV GRU
std::shared_ptr<std::string> LATEST_PROCESSED_CSV_GRU = nullptr;

app.get("/health", []()
        {
    // For a health check route
    return JSONResponse({{"status", "ok"}}); });

app.post("/process_csv", [](UploadFile file, int clusters = Query(2, ge = 2, le = 12))
         {
    // frontend sends a POST request with form data, 'file' field contains a file to analyze
    try {
        // Reads file content to memory
        auto file_bytes = file.read();

        // Calls ML pipeline function to process the file
        auto [summary, csv_bytes] = process_email_csv(file_bytes, clusters);

        // Stores the cleaned CSV in memory for later download
        LATEST_PROCESSED_CSV = std::make_shared<std::string>(csv_bytes);

        // Returns the summary JSON (includes info like cluster info, sample messages)
        return JSONResponse(summary);
    } catch (const std::exception& e) {
        // Returns error
        throw HTTPException(400, "Processing failed: " + std::string(e.what()));
    } });

app.get("/download_csv", []()
        {
    // downloads the most recently processed CSV, called after posting
    if (!LATEST_PROCESSED_CSV) {
        throw HTTPException(404, "No processed CSV available yet.");
    }

    // Streams CSV back to the user to download it
    return StreamingResponse(
        std::make_shared<std::stringstream>(LATEST_PROCESSED_CSV->data()),
        "text/csv",
        {{"Content-Disposition", "attachment; filename=cleaned_emails.csv"}}
    ); });

app.post("/process_csv_gru", [](UploadFile file)
         {
    try {
        auto file_bytes = file.read();
        auto [summary, csv_bytes] = process_email_csv_gru(file_bytes);
        LATEST_PROCESSED_CSV_GRU = std::make_shared<std::string>(csv_bytes);
        return JSONResponse(summary);
    } catch (const std::exception& e) {
        throw HTTPException(400, "GRU processing failed: " + std::string(e.what()));
    } });

app.get("/download_csv_gru", []()
        {
    if (!LATEST_PROCESSED_CSV_GRU) {
        throw HTTPException(404, "No GRU-processed CSV available yet.");
    }
    return StreamingResponse(
        std::make_shared<std::stringstream>(LATEST_PROCESSED_CSV_GRU->data()),
        "text/csv",
        {{"Content-Disposition", "attachment; filename=gru_spam_sorted.csv"}}
    ); });
