
#include "filemanager.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

QString FileManager::open_file_dialog() {
    QString file_path = QFileDialog::getOpenFileName(
        nullptr, "Open CSV File",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV Files (*.csv);;All Files (*.*)");
    return file_path;
}

QByteArray FileManager::read_file(const QString &file_path) {
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

bool FileManager::save_file(const QString &file_path, const QByteArray &data) {
    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(data);
    file.close();
    return true;
}

bool FileManager::file_exists(const QString &file_path) { return QFileInfo::exists(file_path); }
