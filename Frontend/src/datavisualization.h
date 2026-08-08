#ifndef DATAVISUALIZATION_H
#define DATAVISUALIZATION_H

#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QScatterSeries>
#include <QWidget>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

struct PCAPoint {
    double x;
    double y;
    int cluster;
};

struct PieData {
    QString name;
    int value;
};

class DataVisualization : public QWidget {
    Q_OBJECT

   public:
    explicit DataVisualization(QWidget *parent = nullptr);

    void draw_pca_scatter(const std::vector<PCAPoint> &points);
    void draw_pie_chart(const std::vector<PieData> &data);
    void draw_sunburst(const json &sunburst_data);
    void clear();

   private:
    QChartView *chart_view;
    QChart *current_chart;

    QChart *create_pca_scatter_chart(const std::vector<PCAPoint> &points);
    QChart *create_pie_chart(const std::vector<PieData> &data);
    QChart *create_sunburst_chart(const json &data);
};

#endif
