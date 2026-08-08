
#include "mainwindow.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QThread>
#include <QUrl>
#include <QVBoxLayout>
#include <iostream>

#include "pipeline.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), last_method("kmeans") {
    setWindowTitle("Email Clustering Analysis Tool");
    setWindowIcon(QIcon());
    setGeometry(100, 100, 1400, 900);
    setAcceptDrops(true);

    setup_ui();
}

MainWindow::~MainWindow() {}

void MainWindow::setup_ui() {
    central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    QVBoxLayout* main_layout = new QVBoxLayout(central_widget);

    // Title
    QLabel* title = new QLabel("AI Native App for Cybersecurity spam File Analysis");
    QFont title_font = title->font();
    title_font.setPointSize(18);
    title_font.setBold(true);
    title->setFont(title_font);
    main_layout->addWidget(title);

    QLabel* subtitle = new QLabel("Explore interactive charts and data visualisations below");
    QFont subtitle_font = subtitle->font();
    subtitle_font.setPointSize(12);
    subtitle->setFont(subtitle_font);
    main_layout->addWidget(subtitle);

    // File selection area
    QGroupBox* file_group = new QGroupBox("File Selection", this);
    QVBoxLayout* file_layout = new QVBoxLayout();

    file_name_label = new QLabel("No file selected");
    file_layout->addWidget(file_name_label);

    select_file_btn = new QPushButton("Select CSV File");
    select_file_btn->setMinimumHeight(50);
    file_layout->addWidget(select_file_btn);

    file_group->setLayout(file_layout);
    main_layout->addWidget(file_group);

    // Processing options
    QGroupBox* options_group = new QGroupBox("Processing Options", this);
    QHBoxLayout* options_layout = new QHBoxLayout();

    options_layout->addWidget(new QLabel("Number of Clusters:"));
    clusters_spinbox = new QSpinBox();
    clusters_spinbox->setMinimum(2);
    clusters_spinbox->setMaximum(12);
    clusters_spinbox->setValue(2);
    options_layout->addWidget(clusters_spinbox);

    options_layout->addStretch();
    options_group->setLayout(options_layout);
    main_layout->addWidget(options_group);

    // Processing buttons
    QGroupBox* button_group = new QGroupBox("Processing", this);
    QHBoxLayout* button_layout = new QHBoxLayout();

    process_kmeans_btn = new QPushButton("Process (KMeans Clustering)");
    process_kmeans_btn->setMinimumHeight(40);
    button_layout->addWidget(process_kmeans_btn);

    process_gru_btn = new QPushButton("Process (GRU Spam Detection)");
    process_gru_btn->setMinimumHeight(40);
    button_layout->addWidget(process_gru_btn);

    download_csv_btn = new QPushButton("Download CSV");
    download_csv_btn->setMinimumHeight(40);
    download_csv_btn->setEnabled(false);
    button_layout->addWidget(download_csv_btn);

    button_group->setLayout(button_layout);
    main_layout->addWidget(button_group);

    // Progress
    progress_bar = new QProgressBar();
    progress_bar->setVisible(false);
    main_layout->addWidget(progress_bar);

    status_label = new QLabel("Ready");
    main_layout->addWidget(status_label);

    // Visualization tabs
    tab_widget = new QTabWidget();

    pca_viz = new DataVisualization();
    tab_widget->addTab(pca_viz, "PCA Visualization");

    pie_viz = new DataVisualization();
    tab_widget->addTab(pie_viz, "Cluster Distribution");

    sunburst_viz = new DataVisualization();
    tab_widget->addTab(sunburst_viz, "Term Distribution");

    main_layout->addWidget(tab_widget);

    // Results
    QGroupBox* results_group = new QGroupBox("Results", this);
    QVBoxLayout* results_layout = new QVBoxLayout();

    QHBoxLayout* stats_layout = new QHBoxLayout();
    total_label = new QLabel("Total rows: 0");
    clusters_label = new QLabel("Clusters: 0");
    stats_layout->addWidget(total_label);
    stats_layout->addWidget(clusters_label);
    stats_layout->addStretch();
    results_layout->addLayout(stats_layout);

    results_text = new QTextEdit();
    results_text->setReadOnly(true);
    results_text->setMaximumHeight(150);
    results_layout->addWidget(results_text);

    results_group->setLayout(results_layout);
    main_layout->addWidget(results_group);

    // Connect signals
    connect(select_file_btn, &QPushButton::clicked, this, &MainWindow::on_select_file);
    connect(process_kmeans_btn, &QPushButton::clicked, this, &MainWindow::on_process_kmeans);
    connect(process_gru_btn, &QPushButton::clicked, this, &MainWindow::on_process_gru);
    connect(download_csv_btn, &QPushButton::clicked, this, &MainWindow::on_download_csv);
}

void MainWindow::on_select_file() {
    QString file_path = FileManager::open_file_dialog();
    if (!file_path.isEmpty()) {
        current_file_path = file_path;
        QFileInfo file_info(file_path);
        file_name_label->setText("Selected: " + file_info.fileName());
        status_label->setText("File selected: " + file_info.fileName());
    }
}

void MainWindow::on_process_kmeans() {
    if (current_file_path.isEmpty()) {
        show_error("Please select a CSV file first");
        return;
    }

    progress_bar->setVisible(true);
    progress_bar->setValue(0);
    process_kmeans_btn->setEnabled(false);
    process_gru_btn->setEnabled(false);
    status_label->setText("Processing with KMeans...");

    QCoreApplication::processEvents();

    auto result =
        Pipeline::process_kmeans(current_file_path.toStdString(), clusters_spinbox->value());

    if (result.success) {
        last_summary = result.summary;
        last_csv_data = QByteArray(reinterpret_cast<const char*>(result.csv_data.data()),
                                   result.csv_data.size());
        last_method = "kmeans";
        update_visualization(result.summary, "KMeans");
        download_csv_btn->setEnabled(true);
        show_success("KMeans processing completed successfully");
    } else {
        show_error("KMeans processing failed: " + QString::fromStdString(result.error_message));
    }

    progress_bar->setVisible(false);
    process_kmeans_btn->setEnabled(true);
    process_gru_btn->setEnabled(true);
}

void MainWindow::on_process_gru() {
    if (current_file_path.isEmpty()) {
        show_error("Please select a CSV file first");
        return;
    }

    progress_bar->setVisible(true);
    progress_bar->setValue(0);
    process_kmeans_btn->setEnabled(false);
    process_gru_btn->setEnabled(false);
    status_label->setText("Processing with GRU...");

    QCoreApplication::processEvents();

    auto result = Pipeline::process_gru(current_file_path.toStdString());

    if (result.success) {
        last_summary = result.summary;
        last_csv_data = QByteArray(reinterpret_cast<const char*>(result.csv_data.data()),
                                   result.csv_data.size());
        last_method = "gru";
        update_visualization(result.summary, "GRU");
        download_csv_btn->setEnabled(true);
        show_success("GRU spam detection completed successfully");
    } else {
        show_error("GRU processing failed: " + QString::fromStdString(result.error_message));
    }

    progress_bar->setVisible(false);
    process_kmeans_btn->setEnabled(true);
    process_gru_btn->setEnabled(true);
}

void MainWindow::on_download_csv() {
    if (last_csv_data.isEmpty()) {
        show_error("No processed CSV available");
        return;
    }

    QString default_name = (last_method == "gru") ? "gru_spam_sorted.csv" : "cleaned_emails.csv";

    QString file_path =
        QFileDialog::getSaveFileName(this, "Save CSV File", default_name, "CSV Files (*.csv)");

    if (!file_path.isEmpty()) {
        if (FileManager::save_file(file_path, last_csv_data)) {
            show_success("CSV downloaded successfully to: " + file_path);
        } else {
            show_error("Failed to save CSV file");
        }
    }
}

void MainWindow::update_visualization(const json& summary, const QString& method) {
    try {
        // Update stats
        int total = summary["total"];
        int clusters = summary["clusters"];
        total_label->setText(QString("Total rows: %1").arg(total));
        clusters_label->setText(QString("Clusters: %1").arg(clusters));

        // Update PCA scatter
        if (summary.contains("pca") && summary["pca"].contains("points")) {
            std::vector<PCAPoint> points;
            for (const auto& p : summary["pca"]["points"]) {
                points.push_back(
                    {p["x"].get<double>(), p["y"].get<double>(), p["cluster"].get<int>()});
            }
            pca_viz->draw_pca_scatter(points);
        }

        // Update pie chart
        if (summary.contains("counts")) {
            std::vector<PieData> pie_data;
            for (const auto& [key, value] : summary["counts"].items()) {
                pie_data.push_back(
                    {QString("Cluster %1").arg(QString::fromStdString(key)), value.get<int>()});
            }
            pie_viz->draw_pie_chart(pie_data);
        }

        // Update sunburst
        if (summary.contains("sunburst")) {
            sunburst_viz->draw_sunburst(summary["sunburst"]);
        }

        // Update results text
        QString results_str = QString("Processing Method: %1\n").arg(method);
        results_str += QString("Total Messages: %1\n").arg(total);
        results_str += QString("Number of Clusters: %1\n").arg(clusters);

        if (summary.contains("samples")) {
            results_str += "\nSample Messages:\n";
            for (const auto& [cluster, samples] : summary["samples"].items()) {
                results_str += QString("Cluster %1:\n").arg(QString::fromStdString(cluster));
                for (const auto& sample : samples) {
                    results_str +=
                        QString("  - %1\n")
                            .arg(QString::fromStdString(sample.get<std::string>()).left(80));
                }
            }
        }

        results_text->setText(results_str);
        status_label->setText(QString("Processing completed with %1").arg(method));

    } catch (const std::exception& e) {
        show_error("Error updating visualization: " +
                   QString::fromStdString(std::string(e.what())));
    }
}

void MainWindow::show_error(const QString& message) {
    status_label->setText("Error: " + message);
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::show_success(const QString& message) {
    status_label->setText(message);
    QMessageBox::information(this, "Success", message);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mime_data = event->mimeData();

    if (mime_data->hasUrls()) {
        QList<QUrl> urls = mime_data->urls();
        if (!urls.isEmpty()) {
            QString file_path = urls[0].toLocalFile();
            if (file_path.endsWith(".csv")) {
                current_file_path = file_path;
                QFileInfo file_info(file_path);
                file_name_label->setText("Selected: " + file_info.fileName());
                status_label->setText("File selected: " + file_info.fileName());
            }
        }
    }
}
