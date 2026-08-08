#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <memory>

#include "datavisualization.h"
#include "filemanager.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   private slots:
    void on_select_file();
    void on_process_kmeans();
    void on_process_gru();
    void on_download_csv();

   protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

   private:
    void setup_ui();
    void update_visualization(const json &summary, const QString &method);
    void show_error(const QString &message);
    void show_success(const QString &message);

    // UI Components
    QWidget *central_widget;
    QTabWidget *tab_widget;

    // File selection
    QPushButton *select_file_btn;
    QLabel *file_label;
    QLabel *file_name_label;

    // Processing options
    QSpinBox *clusters_spinbox;
    QComboBox *method_combobox;

    // Processing buttons
    QPushButton *process_kmeans_btn;
    QPushButton *process_gru_btn;
    QPushButton *download_csv_btn;

    // Progress
    QProgressBar *progress_bar;
    QLabel *status_label;

    // Results
    QTextEdit *results_text;
    QLabel *total_label;
    QLabel *clusters_label;

    // Visualization tabs
    DataVisualization *pca_viz;
    DataVisualization *pie_viz;
    DataVisualization *sunburst_viz;

    // State
    QString current_file_path;
    QString last_method;
    QByteArray last_csv_data;
    json last_summary;
};

#endif
