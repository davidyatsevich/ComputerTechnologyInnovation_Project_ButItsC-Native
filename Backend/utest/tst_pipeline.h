#ifndef TST_PIPELINE_H
#define TST_PIPELINE_H

#include <QObject>

class TstPipeline : public QObject {
    Q_OBJECT

   private slots:
    void process_kmeans_succeeds_on_valid_file();
    void process_kmeans_fails_gracefully_on_missing_file();
    void process_gru_succeeds_on_valid_file();
    void save_csv_writes_readable_file();
    void save_csv_reports_failure_on_bad_path();
};

#endif
