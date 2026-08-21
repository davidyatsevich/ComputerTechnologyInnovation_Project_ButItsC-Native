#ifndef TST_KMEANSPROCESSOR_H
#define TST_KMEANSPROCESSOR_H

#include <QObject>

class TstKMeansProcessor : public QObject {
    Q_OBJECT

   private slots:
    void process_email_csv_returns_summary_for_every_message();
    void process_email_csv_counts_sum_to_total();
    void process_email_csv_output_csv_has_expected_header();
    void process_email_csv_throws_on_empty_rows();
    void process_email_csv_picks_known_message_column();
};

#endif
