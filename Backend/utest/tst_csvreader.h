#ifndef TST_CSVREADER_H
#define TST_CSVREADER_H

#include <QObject>

class TstCSVReader : public QObject {
    Q_OBJECT

   private slots:
    void parse_simple_csv();
    void parse_quoted_field_with_comma();
    void parse_skips_blank_lines();
    void parse_trims_whitespace();
    void column_index_matches_headers();
    void read_from_bytes_matches_read_from_string();
    void write_csv_round_trip();
    void write_csv_to_bytes_matches_string();
};

#endif
