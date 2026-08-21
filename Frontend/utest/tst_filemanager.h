#ifndef TST_FILEMANAGER_H
#define TST_FILEMANAGER_H

#include <QObject>

class TstFileManager : public QObject {
    Q_OBJECT

   private slots:
    void file_exists_true_for_existing_file();
    void file_exists_false_for_missing_file();
    void read_file_returns_contents();
    void read_file_returns_empty_for_missing_file();
    void save_file_writes_contents();
    void save_file_returns_false_on_bad_path();
    void save_then_read_round_trip();

    // Note: open_file_dialog() is not covered here - it blocks on a native,
    // interactive QFileDialog and has no meaningful headless behavior to
    // assert on.
};

#endif
