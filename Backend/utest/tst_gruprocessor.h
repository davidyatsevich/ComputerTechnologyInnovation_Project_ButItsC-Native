#ifndef TST_GRUPROCESSOR_H
#define TST_GRUPROCESSOR_H

#include <QObject>

class TstGRUProcessor : public QObject {
    Q_OBJECT

   private slots:
    void process_email_csv_gru_returns_summary_for_every_message();
    void process_email_csv_gru_flags_obvious_spam();
    void process_email_csv_gru_counts_sum_to_total();
    void process_email_csv_gru_throws_on_empty_rows();
    void process_email_csv_gru_falls_back_to_first_column();
};

#endif
