#include "tst_filemanager.h"

#include <QtTest>

#include "filemanager.h"

void TstFileManager::file_exists_true_for_existing_file() {
    QTemporaryFile file;
    QVERIFY(file.open());
    QString path = file.fileName();

    QVERIFY(FileManager::file_exists(path));
}

void TstFileManager::file_exists_false_for_missing_file() {
    QVERIFY(!FileManager::file_exists("/this/path/does/not/exist.csv"));
}

void TstFileManager::read_file_returns_contents() {
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("hello,world\n1,2\n");
    QString path = file.fileName();
    file.close();

    QByteArray contents = FileManager::read_file(path);

    QCOMPARE(contents, QByteArray("hello,world\n1,2\n"));
}

void TstFileManager::read_file_returns_empty_for_missing_file() {
    QByteArray contents = FileManager::read_file("/this/path/does/not/exist.csv");

    QVERIFY(contents.isEmpty());
}

void TstFileManager::save_file_writes_contents() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/saved.csv";

    bool ok = FileManager::save_file(path, QByteArray("a,b\n1,2\n"));

    QVERIFY(ok);
    QVERIFY(QFile::exists(path));
}

void TstFileManager::save_file_returns_false_on_bad_path() {
    bool ok = FileManager::save_file("/this/directory/does/not/exist/out.csv",
                                     QByteArray("data"));

    QVERIFY(!ok);
}

void TstFileManager::save_then_read_round_trip() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/roundtrip.csv";
    QByteArray original("email,clusterid\na@b.com,0\n");

    QVERIFY(FileManager::save_file(path, original));
    QByteArray read_back = FileManager::read_file(path);

    QCOMPARE(read_back, original);
}

QTEST_MAIN(TstFileManager)
