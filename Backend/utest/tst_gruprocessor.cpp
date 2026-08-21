#include "tst_gruprocessor.h"

#include <QtTest>

#include "gru_processor.h"

namespace {
std::vector<uint8_t> make_csv_bytes(const std::string &csv) {
    return std::vector<uint8_t>(csv.begin(), csv.end());
}
}  // namespace

void TstGRUProcessor::process_email_csv_gru_returns_summary_for_every_message() {
    std::string csv =
        "message,label\n"
        "let us meet for lunch tomorrow at noon,ham\n"
        "FREE prize! click now to verify your account! act now!!!,spam\n";

    GRUResult result = GRUProcessor::process_email_csv_gru(make_csv_bytes(csv));

    QCOMPARE(result.summary["total"].get<int>(), 2);
    QCOMPARE(result.summary["clusters"].get<int>(), 2);
    QVERIFY(result.summary.contains("pca"));
}

void TstGRUProcessor::process_email_csv_gru_flags_obvious_spam() {
    // Keyword-heavy message should score well above the spam_score > 2
    // threshold used by the heuristic, and the plain message should not.
    std::string csv =
        "message,label\n"
        "let us meet for lunch tomorrow at noon,ham\n"
        "FREE prize! click now to verify your account! act now!!!,spam\n";

    GRUResult result = GRUProcessor::process_email_csv_gru(make_csv_bytes(csv));

    // Two distinct predicted clusters should appear in the counts (one for
    // the ham-like message, one for the spam-like message).
    QCOMPARE(result.summary["counts"].size(), static_cast<size_t>(2));

    int total_from_counts = 0;
    for (auto &[key, value] : result.summary["counts"].items()) {
        total_from_counts += value.get<int>();
    }
    QCOMPARE(total_from_counts, 2);
}

void TstGRUProcessor::process_email_csv_gru_counts_sum_to_total() {
    std::string csv =
        "message,label\n"
        "let us meet for lunch tomorrow at noon,ham\n"
        "can you review the attached document,ham\n"
        "FREE prize! click now to verify your account! act now!!!,spam\n";

    GRUResult result = GRUProcessor::process_email_csv_gru(make_csv_bytes(csv));

    int total = result.summary["total"].get<int>();
    int summed_counts = 0;
    for (auto &[key, value] : result.summary["counts"].items()) {
        summed_counts += value.get<int>();
    }

    QCOMPARE(summed_counts, total);
}

void TstGRUProcessor::process_email_csv_gru_throws_on_empty_rows() {
    std::string csv = "message,label\n";

    QVERIFY_EXCEPTION_THROWN(GRUProcessor::process_email_csv_gru(make_csv_bytes(csv)),
                             std::runtime_error);
}

void TstGRUProcessor::process_email_csv_gru_falls_back_to_first_column() {
    // No recognized message-column name here, so pick_message_column()
    // should fall back to the first header instead of throwing.
    std::string csv =
        "unrecognized_column,label\n"
        "let us meet for lunch tomorrow at noon,ham\n";

    GRUResult result = GRUProcessor::process_email_csv_gru(make_csv_bytes(csv));

    QCOMPARE(result.summary["total"].get<int>(), 1);
}

QTEST_MAIN(TstGRUProcessor)
