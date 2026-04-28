#pragma once

#include "movie_table_model.h"
#include "movie_service.h"
#include "csv_importer.h"
#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableView>
#include <QLabel>
#include <QListView>
#include <QStringListModel>
#include <QComboBox>
#include <QProgressBar>
#include <QFutureWatcher>
#include <memory>

namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onSearchClicked();
    void onResetClicked();
    void onImportClicked();
    void onMovieSelected(const QModelIndex& index);
    void onImportProgress(int percent);
    void onImportFinished();
    void onImportError(QString message);
    void onSortChanged(int index);
    void onSortOrderClicked();

private:
    void setupUi();
    void refreshTable();
    void showMovieDetails(int movieId);
    void clearDetails();

    data::MovieRepository m_repo;
    service::MovieService m_service;

    // Search / filters
    QLineEdit* m_searchEdit = nullptr;
    QSpinBox* m_minYearSpin = nullptr;
    QSpinBox* m_maxYearSpin = nullptr;
    QSpinBox* m_minLengthSpin = nullptr;
    QSpinBox* m_maxLengthSpin = nullptr;
    QPushButton* m_searchBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;
    QPushButton* m_importBtn = nullptr;
    QComboBox* m_sortCombo = nullptr;
    QPushButton* m_sortOrderBtn = nullptr;

    core::SortField m_currentSortField = core::SortField::Title;
    core::SortOrder m_currentSortOrder = core::SortOrder::Asc;

    // Table
    QTableView* m_tableView = nullptr;
    MovieTableModel* m_tableModel = nullptr;

    // Details
    QLabel* m_detailTitle = nullptr;
    QLabel* m_detailYear = nullptr;
    QLabel* m_detailLength = nullptr;
    QLabel* m_detailSlogan = nullptr;
    QListView* m_castList = nullptr;
    QStringListModel* m_castModel = nullptr;
    QLabel* m_imageLabel = nullptr;

    // Progress
    QProgressBar* m_progressBar = nullptr;

    // Async import
    std::unique_ptr<importer::CsvImporter> m_importer;
    QFutureWatcher<void>* m_importWatcher = nullptr;
};

} // namespace ui
