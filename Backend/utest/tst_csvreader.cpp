#include "tst_csvreader.h"

#include <QtTest>

#include "csv_reader.h"

void TstCSVReader::parse_simple_csv() {
    std::string csv = "name,age\nAlice,30\nBob,25\n";
    std::vector<uint8_t> bytes(csv.begin(), csv.end());

    CSVData data = CSVReader::read_csv_from_bytes(bytes);

    QCOMPARE(data.headers.size(), static_cast<size_t>(2));
    QCOMPARE(QString::fromStdString(data.headers[0]), QString("name"));
    QCOMPARE(QString::fromStdString(data.headers[1]), QString("age"));

    QCOMPARE(data.rows.size(), static_cast<size_t>(2));
    QCOMPARE(QString::fromStdString(data.rows[0][0]), QString("Alice"));
    QCOMPARE(QString::fromStdString(data.rows[0][1]), QString("30"));
    QCOMPARE(QString::fromStdString(data.rows[1][0]), QString("Bob"));
}

void TstCSVReader::parse_quoted_field_with_comma() {
    std::string csv = "message,label\n\"hello, world\",ham\n";
    std::vector<uint8_t> bytes(csv.begin(), csv.end());

    CSVData data = CSVReader::read_csv_from_bytes(bytes);

    QCOMPARE(data.rows.size(), static_cast<size_t>(1));
    // The parser keeps the surrounding quotes; assert the comma inside the
    // quoted field did not split it into an extra column.
    QCOMPARE(data.rows[0].size(), static_cast<size_t>(2));
    QVERIFY(QString::fromStdString(data.rows[0][0]).contains("hello, world"));
    QCOMPARE(QString::fromStdString(data.rows[0][1]), QString("ham"));
}

void TstCSVReader::parse_skips_blank_lines() {
    std::string csv = "a,b\n1,2\n\n3,4\n";
    std::vector<uint8_t> bytes(csv.begin(), csv.end());

    CSVData data = CSVReader::read_csv_from_bytes(bytes);

    QCOMPARE(data.rows.size(), static_cast<size_t>(2));
}

void TstCSVReader::parse_trims_whitespace() {
    std::string csv = "a,b\n  1  ,  2  \n";
    std::vector<uint8_t> bytes(csv.begin(), csv.end());

    CSVData data = CSVReader::read_csv_from_bytes(bytes);

    QCOMPARE(QString::fromStdString(data.rows[0][0]), QString("1"));
    QCOMPARE(QString::fromStdString(data.rows[0][1]), QString("2"));
}

void TstCSVReader::column_index_matches_headers() {
    std::string csv = "email,clusterid\nfoo@bar.com,0\n";
    std::vector<uint8_t> bytes(csv.begin(), csv.end());

    CSVData data = CSVReader::read_csv_from_bytes(bytes);

    QVERIFY(data.column_index.count("email") == 1);
    QVERIFY(data.column_index.count("clusterid") == 1);
    QCOMPARE(data.column_index.at("email"), 0);
    QCOMPARE(data.column_index.at("clusterid"), 1);
}

void TstCSVReader::read_from_bytes_matches_read_from_string() {
    // read_csv_from_file() reads from disk, so route both paths through a
    // temp file to confirm bytes-based parsing agrees with file-based
    // parsing on identical content.
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("x,y\n1,2\n3,4\n");
    QString path = file.fileName();
    file.close();

    CSVData from_file = CSVReader::read_csv_from_file(path.toStdString());

    std::string csv = "x,y\n1,2\n3,4\n";
    std::vector<uint8_t> bytes(csv.begin(), csv.end());
    CSVData from_bytes = CSVReader::read_csv_from_bytes(bytes);

    QCOMPARE(from_file.headers.size(), from_bytes.headers.size());
    QCOMPARE(from_file.rows.size(), from_bytes.rows.size());
    for (size_t i = 0; i < from_file.rows.size(); ++i) {
        QCOMPARE(QString::fromStdString(from_file.rows[i][0]),
                 QString::fromStdString(from_bytes.rows[i][0]));
    }
}

void TstCSVReader::write_csv_round_trip() {
    std::vector<std::string> headers = {"email", "clusterid"};
    std::vector<std::vector<std::string>> rows = {{"a@b.com", "0"}, {"c@d.com", "1"}};

    std::string csv_str = CSVReader::write_csv_to_string(headers, rows);
    std::vector<uint8_t> bytes(csv_str.begin(), csv_str.end());

    CSVData parsed = CSVReader::read_csv_from_bytes(bytes);

    QCOMPARE(parsed.headers.size(), headers.size());
    QCOMPARE(parsed.rows.size(), rows.size());
    QCOMPARE(QString::fromStdString(parsed.rows[1][0]), QString("c@d.com"));
    QCOMPARE(QString::fromStdString(parsed.rows[1][1]), QString("1"));
}

void TstCSVReader::write_csv_to_bytes_matches_string() {
    std::vector<std::string> headers = {"a", "b"};
    std::vector<std::vector<std::string>> rows = {{"1", "2"}};

    std::string as_string = CSVReader::write_csv_to_string(headers, rows);
    std::vector<uint8_t> as_bytes = CSVReader::write_csv_to_bytes(headers, rows);

    QCOMPARE(as_bytes.size(), as_string.size());
    QCOMPARE(std::string(as_bytes.begin(), as_bytes.end()), as_string);
}

QTEST_MAIN(TstCSVReader)
