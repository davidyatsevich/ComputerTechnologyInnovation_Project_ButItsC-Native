#ifndef TST_DATAVISUALIZATION_H
#define TST_DATAVISUALIZATION_H

#include <QObject>

class TstDataVisualization : public QObject {
    Q_OBJECT

   private slots:
    void construction_creates_embedded_chart_view();
    void draw_pca_scatter_creates_one_series_per_cluster();
    void draw_pca_scatter_handles_empty_points();
    void draw_pie_chart_creates_slice_per_entry();
    void draw_pie_chart_handles_empty_data();
    void draw_sunburst_does_not_crash();
    void clear_resets_to_empty_chart();
};

#endif
