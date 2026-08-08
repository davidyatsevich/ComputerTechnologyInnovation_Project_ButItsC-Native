
#ifndef FILEMANAGER_H

#define FILEMANAGER_H

#include <QMap>
#include <QString>
#include <QVector>

class FileManager {
   public:
    static QString open_file_dialog();
    static QByteArray read_file(const QString &file_path);
    static bool save_file(const QString &file_path, const QByteArray &data);
    static bool file_exists(const QString &file_path);
};

#endif