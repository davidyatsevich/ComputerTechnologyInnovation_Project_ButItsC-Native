#include "datavisualization.h"

#include <QColor>
#include <QDebug>
#include <QPieSeries>
#include <QPieSlice>
#include <QRandomGenerator>
#include <QScatterSeries>
#include <QVBoxLayout>
#include <QValueAxis>

DataVisualization::DataVisualization(QWidget *parent) : QWidget(parent), current_chart(nullptr) {
    chart_view = new QChartView(this);
    chart_view->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chart_view);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

QChart *DataVisualization::create_pca_scatter_chart(const std::vector<PCAPoint> &points) {
    QChart *chart = new QChart();
    chart->setTitle("PCA Visualization");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    if (points.empty()) {
        chart->setTitle("PCA Visualization (No Data)");
        return chart;
    }

    // Group points by cluster
    std::map<int, std::vector<PCAPoint>> clusters;
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;

    for (const auto &point : points) {
        clusters[point.cluster].push_back(point);
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }

    // Define colors for clusters
    std::vector<QColor> colors = {QColor(Qt::red),     QColor(Qt::blue),    QColor(Qt::green),
                                  QColor(Qt::yellow),  QColor(Qt::cyan),    QColor(Qt::magenta),
                                  QColor(Qt::gray),    QColor(Qt::darkRed), QColor(Qt::darkGreen),
                                  QColor(Qt::darkBlue)};

    // Create series for each cluster
    int color_idx = 0;
    for (const auto &[cluster_id, cluster_points] : clusters) {
        QScatterSeries *series = new QScatterSeries();
        series->setName(QString("Cluster %1").arg(cluster_id));
        series->setColor(colors[color_idx % colors.size()]);
        series->setMarkerSize(5);

        for (const auto &point : cluster_points) {
            series->append(point.x, point.y);
        }

        chart->addSeries(series);
        color_idx++;
    }

    // Create axes
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("PC1");
    axisX->setRange(min_x, max_x);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("PC2");
    axisY->setRange(min_y, max_y);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Attach axes to series
    for (auto series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    return chart;
}

QChart *DataVisualization::create_pie_chart(const std::vector<PieData> &data) {
    QChart *chart = new QChart();
    chart->setTitle("Cluster Distribution");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    if (data.empty()) {
        chart->setTitle("Cluster Distribution (No Data)");
        return chart;
    }

    QPieSeries *series = new QPieSeries();

    std::vector<QColor> colors = {QColor(Qt::red),    QColor(Qt::blue), QColor(Qt::green),
                                  QColor(Qt::yellow), QColor(Qt::cyan), QColor(Qt::magenta)};

    int color_idx = 0;
    for (const auto &item : data) {
        QPieSlice *slice = new QPieSlice(item.name, item.value);
        slice->setColor(colors[color_idx % colors.size()]);
        slice->setLabelVisible(true);
        series->append(slice);
        color_idx++;
    }

    chart->addSeries(series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    return chart;
}

QChart *DataVisualization::create_sunburst_chart(const json &data) {
    // Simplified sunburst visualization - showing as a tree structure
    QChart *chart = new QChart();
    chart->setTitle("Term Distribution by Cluster");

    // For now, we'll display a simple text representation
    // A full sunburst would require custom painting

    return chart;
}

void DataVisualization::draw_pca_scatter(const std::vector<PCAPoint> &points) {
    // Note: QChartView::setChart() takes ownership of the new chart and
    // automatically deletes the previously-set chart. Manually deleting
    // current_chart here (before calling setChart) would cause a double-free,
    // since setChart() would then try to delete the same already-freed
    // pointer it still remembers internally.
    current_chart = create_pca_scatter_chart(points);
    chart_view->setChart(current_chart);
}

void DataVisualization::draw_pie_chart(const std::vector<PieData> &data) {
    current_chart = create_pie_chart(data);
    chart_view->setChart(current_chart);
}

void DataVisualization::draw_sunburst(const json &sunburst_data) {
    current_chart = create_sunburst_chart(sunburst_data);
    chart_view->setChart(current_chart);
}

void DataVisualization::clear() {
    current_chart = new QChart();
    chart_view->setChart(current_chart);
}
