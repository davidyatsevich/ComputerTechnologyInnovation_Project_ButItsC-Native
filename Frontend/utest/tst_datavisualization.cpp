#include "tst_datavisualization.h"

#include <QChartView>
#include <QPieSeries>
#include <QtTest>

#include "datavisualization.h"

void TstDataVisualization::construction_creates_embedded_chart_view() {
    DataVisualization viz;

    QChartView *chart_view = viz.findChild<QChartView *>();
    QVERIFY(chart_view != nullptr);
    QVERIFY(chart_view->chart() != nullptr);
}

void TstDataVisualization::draw_pca_scatter_creates_one_series_per_cluster() {
    DataVisualization viz;
    std::vector<PCAPoint> points = {
        {0.1, 0.2, 0}, {0.3, 0.1, 0}, {-0.2, -0.1, 1}, {-0.1, 0.4, 1}, {0.5, 0.5, 2},
    };

    viz.draw_pca_scatter(points);

    QChartView *chart_view = viz.findChild<QChartView *>();
    QVERIFY(chart_view != nullptr);
    QCOMPARE(chart_view->chart()->title(), QString("PCA Visualization"));
    QCOMPARE(chart_view->chart()->series().size(), 3);
}

void TstDataVisualization::draw_pca_scatter_handles_empty_points() {
    DataVisualization viz;

    viz.draw_pca_scatter({});

    QChartView *chart_view = viz.findChild<QChartView *>();
    QVERIFY(chart_view->chart()->title().contains("No Data"));
    QVERIFY(chart_view->chart()->series().isEmpty());
}

void TstDataVisualization::draw_pie_chart_creates_slice_per_entry() {
    DataVisualization viz;
    std::vector<PieData> data = {
        {"Cluster 0", 5},
        {"Cluster 1", 3},
    };

    viz.draw_pie_chart(data);

    QChartView *chart_view = viz.findChild<QChartView *>();
    QCOMPARE(chart_view->chart()->title(), QString("Cluster Distribution"));
    QCOMPARE(chart_view->chart()->series().size(), 1);

    auto *series = qobject_cast<QPieSeries *>(chart_view->chart()->series().first());
    QVERIFY(series != nullptr);
    QCOMPARE(series->slices().size(), 2);
}

void TstDataVisualization::draw_pie_chart_handles_empty_data() {
    DataVisualization viz;

    viz.draw_pie_chart({});

    QChartView *chart_view = viz.findChild<QChartView *>();
    QVERIFY(chart_view->chart()->title().contains("No Data"));
    QVERIFY(chart_view->chart()->series().isEmpty());
}

void TstDataVisualization::draw_sunburst_does_not_crash() {
    DataVisualization viz;
    json data;
    data["name"] = "root";
    data["children"] = json::array();

    viz.draw_sunburst(data);

    QChartView *chart_view = viz.findChild<QChartView *>();
    QVERIFY(chart_view->chart() != nullptr);
}

void TstDataVisualization::clear_resets_to_empty_chart() {
    DataVisualization viz;
    viz.draw_pie_chart({{"Cluster 0", 1}});

    viz.clear();

    QChartView *chart_view = viz.findChild<QChartView *>();
    QVERIFY(chart_view->chart()->series().isEmpty());
}

QTEST_MAIN(TstDataVisualization)
