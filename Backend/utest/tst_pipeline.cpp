#include "tst_pipeline.h"

#include <QtTest>

#include "pipeline.h"

void TstPipeline::process_kmeans_succeeds_on_valid_file() {
    QTemporaryFile file(QDir::tempPath() + "/tst_pipeline_XXXXXX.csv");
    QVERIFY(file.open());
    file.write(
        "message,label\n"
        "win a free prize now,spam\n"
        "meeting moved to 3pm tomorrow,ham\n"
        "call to claim your reward today,spam\n"
        "can you review the attached document,ham\n");
    QString path = file.fileName();
    file.close();

    ProcessingResult result = Pipeline::process_kmeans(path.toStdString(), 2);

    QVERIFY(result.success);
    QVERIFY(result.error_message.empty());
    QCOMPARE(result.summary["total"].get<int>(), 4);
    QVERIFY(!result.csv_data.empty());
}

void TstPipeline::process_kmeans_fails_gracefully_on_missing_file() {
    ProcessingResult result =
        Pipeline::process_kmeans("/nonexistent/path/does_not_exist.csv", 2);

    QVERIFY(!result.success);
    QVERIFY(!result.error_message.empty());
}

void TstPipeline::process_gru_succeeds_on_valid_file() {
    QTemporaryFile file(QDir::tempPath() + "/tst_pipeline_gru_XXXXXX.csv");
    QVERIFY(file.open());
    file.write(
        "message,label\n"
        "let us meet for lunch tomorrow,ham\n"
        "FREE prize! click now to verify your account! act now!!!,spam\n");
    QString path = file.fileName();
    file.close();

    ProcessingResult result = Pipeline::process_gru(path.toStdString());

    QVERIFY(result.success);
    QCOMPARE(result.summary["total"].get<int>(), 2);
    QVERIFY(!result.csv_data.empty());
}

void TstPipeline::save_csv_writes_readable_file() {
    std::vector<uint8_t> data = {'a', ',', 'b', '\n', '1', ',', '2', '\n'};

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    std::string out_path = (dir.path() + "/out.csv").toStdString();

    std::string message = Pipeline::save_csv(data, out_path);

    QVERIFY(QString::fromStdString(message).contains("successfully"));
    QVERIFY(QFile::exists(QString::fromStdString(out_path)));

    QFile written(QString::fromStdString(out_path));
    QVERIFY(written.open(QIODevice::ReadOnly));
    QCOMPARE(written.readAll(), QByteArray("a,b\n1,2\n"));
}

void TstPipeline::save_csv_reports_failure_on_bad_path() {
    std::vector<uint8_t> data = {'x'};

    std::string message =
        Pipeline::save_csv(data, "/this/directory/does/not/exist/out.csv");

    QVERIFY(QString::fromStdString(message).contains("Failed"));
}

QTEST_MAIN(TstPipeline)
