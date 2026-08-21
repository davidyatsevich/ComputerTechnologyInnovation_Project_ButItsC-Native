#include "tst_kmeansprocessor.h"

#include <QtTest>

#include "kmeans_processor.h"

namespace {
std::vector<uint8_t> make_csv_bytes(const std::string &csv) {
    return std::vector<uint8_t>(csv.begin(), csv.end());
}
}  // namespace

void TstKMeansProcessor::process_email_csv_returns_summary_for_every_message() {
    std::string csv =
        "message,label\n"
        "win a free prize now,spam\n"
        "call this number to claim your reward,spam\n"
        "meeting moved to 3pm tomorrow,ham\n"
        "can you review the attached document,ham\n"
        "congratulations you have been selected,spam\n"
        "lunch on friday sounds good,ham\n";

    KMeansResult result = KMeansProcessor::process_email_csv(make_csv_bytes(csv), 2);

    QCOMPARE(result.summary["total"].get<int>(), 6);
    QCOMPARE(result.summary["clusters"].get<int>(), 2);
    QVERIFY(result.summary.contains("pca"));
    QCOMPARE(result.summary["pca"]["points"].size(), static_cast<size_t>(6));
}

void TstKMeansProcessor::process_email_csv_counts_sum_to_total() {
    std::string csv =
        "message,label\n"
        "win a free prize now,spam\n"
        "call this number to claim your reward,spam\n"
        "meeting moved to 3pm tomorrow,ham\n"
        "can you review the attached document,ham\n";

    KMeansResult result = KMeansProcessor::process_email_csv(make_csv_bytes(csv), 2);

    int total = result.summary["total"].get<int>();
    int summed_counts = 0;
    for (auto &[key, value] : result.summary["counts"].items()) {
        summed_counts += value.get<int>();
    }

    QCOMPARE(summed_counts, total);
}

void TstKMeansProcessor::process_email_csv_output_csv_has_expected_header() {
    std::string csv =
        "message,label\n"
        "win a free prize now,spam\n"
        "meeting moved to 3pm tomorrow,ham\n"
        "can you review the attached document,ham\n";

    KMeansResult result = KMeansProcessor::process_email_csv(make_csv_bytes(csv), 2);

    std::string out(result.csv_bytes.begin(), result.csv_bytes.end());
    QVERIFY(QString::fromStdString(out).startsWith("email,clusterid"));

    // One data row per input message, plus the header row.
    int line_count = QString::fromStdString(out).split('\n', Qt::SkipEmptyParts).size();
    QCOMPARE(line_count, 4);
}

void TstKMeansProcessor::process_email_csv_throws_on_empty_rows() {
    std::string csv = "message,label\n";

    QVERIFY_EXCEPTION_THROWN(KMeansProcessor::process_email_csv(make_csv_bytes(csv), 2),
                             std::runtime_error);
}

void TstKMeansProcessor::process_email_csv_picks_known_message_column() {
    // "text" is one of the recognized message-column candidates, so this
    // should process successfully even though the column isn't named
    // "message".
    std::string csv =
        "text,label\n"
        "win a free prize now,spam\n"
        "meeting moved to 3pm tomorrow,ham\n";

    KMeansResult result = KMeansProcessor::process_email_csv(make_csv_bytes(csv), 2);

    QCOMPARE(result.summary["total"].get<int>(), 2);
}

QTEST_MAIN(TstKMeansProcessor)
