#include "main_window.h"
#include "movie_service.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QPixmap>
#include <filesystem>

namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_repo("movies.db")
    , m_service(m_repo)
{
    setupUi();
    refreshTable();

    m_importer = std::make_unique<importer::CsvImporter>(m_repo);
    connect(m_importer.get(), &importer::CsvImporter::progressChanged,
            this, &MainWindow::onImportProgress);
    connect(m_importer.get(), &importer::CsvImporter::finished,
            this, &MainWindow::onImportFinished);
    connect(m_importer.get(), &importer::CsvImporter::error,
            this, &MainWindow::onImportError);

    m_importWatcher = new QFutureWatcher<void>(this);
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QHBoxLayout(central);

    // ===== Left panel: filters + table =====
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);

    // Filters group
    auto* filterGroup = new QGroupBox(QStringLiteral("Search & Filters"), this);
    auto* filterGrid = new QGridLayout(filterGroup);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search by title, actor, role..."));
    filterGrid->addWidget(new QLabel(QStringLiteral("Search:")), 0, 0);
    filterGrid->addWidget(m_searchEdit, 0, 1, 1, 3);

    m_minYearSpin = new QSpinBox(this);
    m_minYearSpin->setRange(0, 9999);
    m_minYearSpin->setSpecialValueText(QStringLiteral("Any"));
    m_maxYearSpin = new QSpinBox(this);
    m_maxYearSpin->setRange(0, 9999);
    m_maxYearSpin->setSpecialValueText(QStringLiteral("Any"));
    m_minLengthSpin = new QSpinBox(this);
    m_minLengthSpin->setRange(0, 999);
    m_minLengthSpin->setSpecialValueText(QStringLiteral("Any"));
    m_maxLengthSpin = new QSpinBox(this);
    m_maxLengthSpin->setRange(0, 999);
    m_maxLengthSpin->setSpecialValueText(QStringLiteral("Any"));

    filterGrid->addWidget(new QLabel(QStringLiteral("Min Year:")), 1, 0);
    filterGrid->addWidget(m_minYearSpin, 1, 1);
    filterGrid->addWidget(new QLabel(QStringLiteral("Max Year:")), 1, 2);
    filterGrid->addWidget(m_maxYearSpin, 1, 3);
    filterGrid->addWidget(new QLabel(QStringLiteral("Min Length:")), 2, 0);
    filterGrid->addWidget(m_minLengthSpin, 2, 1);
    filterGrid->addWidget(new QLabel(QStringLiteral("Max Length:")), 2, 2);
    filterGrid->addWidget(m_maxLengthSpin, 2, 3);

    m_searchBtn = new QPushButton(QStringLiteral("Search"), this);
    m_resetBtn = new QPushButton(QStringLiteral("Reset"), this);
    m_importBtn = new QPushButton(QStringLiteral("Import CSV"), this);

    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItem(QStringLiteral("Title"), QVariant::fromValue(static_cast<int>(core::SortField::Title)));
    m_sortCombo->addItem(QStringLiteral("Year"), QVariant::fromValue(static_cast<int>(core::SortField::Year)));
    m_sortCombo->addItem(QStringLiteral("Length"), QVariant::fromValue(static_cast<int>(core::SortField::Length)));

    m_sortOrderBtn = new QPushButton(QStringLiteral("Asc"), this);
    m_sortOrderBtn->setFixedWidth(50);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_searchBtn);
    btnLayout->addWidget(m_resetBtn);
    btnLayout->addWidget(m_importBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(new QLabel(QStringLiteral("Sort:")));
    btnLayout->addWidget(m_sortCombo);
    btnLayout->addWidget(m_sortOrderBtn);
    filterGrid->addLayout(btnLayout, 3, 0, 1, 4);

    leftLayout->addWidget(filterGroup);

    // Table
    m_tableModel = new MovieTableModel(this);
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setAlternatingRowColors(true);
    leftLayout->addWidget(m_tableView);

    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setVisible(false);
    leftLayout->addWidget(m_progressBar);

    mainLayout->addWidget(leftPanel, 2);

    // ===== Right panel: details =====
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);

    auto* detailsGroup = new QGroupBox(QStringLiteral("Movie Details"), this);
    auto* detailsLayout = new QFormLayout(detailsGroup);

    m_detailTitle = new QLabel(QStringLiteral("-"), this);
    m_detailYear = new QLabel(QStringLiteral("-"), this);
    m_detailLength = new QLabel(QStringLiteral("-"), this);
    m_detailSlogan = new QLabel(QStringLiteral("-"), this);
    m_detailSlogan->setWordWrap(true);

    detailsLayout->addRow(QStringLiteral("Title:"), m_detailTitle);
    detailsLayout->addRow(QStringLiteral("Year:"), m_detailYear);
    detailsLayout->addRow(QStringLiteral("Length:"), m_detailLength);
    detailsLayout->addRow(QStringLiteral("Slogan:"), m_detailSlogan);

    rightLayout->addWidget(detailsGroup);

    auto* castGroup = new QGroupBox(QStringLiteral("Cast"), this);
    auto* castLayout = new QVBoxLayout(castGroup);
    m_castModel = new QStringListModel(this);
    m_castList = new QListView(this);
    m_castList->setModel(m_castModel);
    castLayout->addWidget(m_castList);
    rightLayout->addWidget(castGroup);

    auto* imageGroup = new QGroupBox(QStringLiteral("Image"), this);
    auto* imageLayout = new QVBoxLayout(imageGroup);
    auto* imageCenterLayout = new QHBoxLayout();
    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(200, 280);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    imageCenterLayout->addStretch();
    imageCenterLayout->addWidget(m_imageLabel);
    imageCenterLayout->addStretch();
    imageLayout->addLayout(imageCenterLayout);
    rightLayout->addWidget(imageGroup);

    rightLayout->addStretch();
    mainLayout->addWidget(rightPanel, 1);

    // Connections
    connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(m_importBtn, &QPushButton::clicked, this, &MainWindow::onImportClicked);
    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onMovieSelected);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortChanged);
    connect(m_sortOrderBtn, &QPushButton::clicked, this, &MainWindow::onSortOrderClicked);

    setWindowTitle(QStringLiteral("Movie App"));
    resize(1000, 700);
}

void MainWindow::refreshTable() {
    auto movies = m_service.getAllMovies(m_currentSortField, m_currentSortOrder);
    m_tableModel->setMovies(std::move(movies));
    m_tableView->resizeColumnsToContents();
}

void MainWindow::onSearchClicked() {
    core::MovieFilters filters;
    if (m_minYearSpin->value() > 0) filters.min_year = m_minYearSpin->value();
    if (m_maxYearSpin->value() > 0) filters.max_year = m_maxYearSpin->value();
    if (m_minLengthSpin->value() > 0) filters.min_length = m_minLengthSpin->value();
    if (m_maxLengthSpin->value() > 0) filters.max_length = m_maxLengthSpin->value();

    std::string query = m_searchEdit->text().toStdString();
    auto movies = m_service.searchAndFilter(query, filters, m_currentSortField, m_currentSortOrder);
    m_tableModel->setMovies(std::move(movies));
    m_tableView->resizeColumnsToContents();
    clearDetails();
}

void MainWindow::onResetClicked() {
    m_searchEdit->clear();
    m_minYearSpin->setValue(0);
    m_maxYearSpin->setValue(0);
    m_minLengthSpin->setValue(0);
    m_maxLengthSpin->setValue(0);
    m_sortCombo->setCurrentIndex(0);
    m_currentSortField = core::SortField::Title;
    m_currentSortOrder = core::SortOrder::Asc;
    m_sortOrderBtn->setText(QStringLiteral("Asc"));
    refreshTable();
    clearDetails();
}

void MainWindow::onImportClicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
        QStringLiteral("Select CSV Data Folder"),
        QString::fromStdString(std::filesystem::current_path().string()));
    if (dir.isEmpty()) return;

    m_importBtn->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);

    std::filesystem::path path = dir.toStdString();
    QFuture<void> future = QtConcurrent::run([this, path]() {
        m_importer->import(path);
    });
    m_importWatcher->setFuture(future);
}

void MainWindow::onImportProgress(int percent) {
    m_progressBar->setValue(percent);
}

void MainWindow::onImportFinished() {
    m_progressBar->setVisible(false);
    m_importBtn->setEnabled(true);
    refreshTable();
    QMessageBox::information(this, QStringLiteral("Import"), QStringLiteral("CSV import completed successfully!"));
}

void MainWindow::onImportError(QString message) {
    m_progressBar->setVisible(false);
    m_importBtn->setEnabled(true);
    QMessageBox::critical(this, QStringLiteral("Import Error"), message);
}

void MainWindow::onMovieSelected(const QModelIndex& index) {
    if (!index.isValid()) return;
    auto idOpt = m_tableModel->movieIdAt(index.row());
    if (idOpt) {
        showMovieDetails(*idOpt);
    }
}

void MainWindow::showMovieDetails(int movieId) {
    auto opt = m_service.getMovieById(movieId);
    if (!opt) {
        clearDetails();
        return;
    }
    const auto& movie = *opt;

    m_detailTitle->setText(QString::fromStdString(movie.title));
    m_detailYear->setText(QString::number(movie.year));
    m_detailLength->setText(QStringLiteral("%1 min").arg(movie.length));
    m_detailSlogan->setText(QString::fromStdString(movie.slogan));

    QStringList castList;
    for (const auto& c : movie.cast) {
        castList << QStringLiteral("%1 as %2")
            .arg(QString::fromStdString(c.actor))
            .arg(QString::fromStdString(c.role));
    }
    m_castModel->setStringList(castList);

    if (!movie.image.empty()) {
        std::filesystem::path root = std::filesystem::current_path();
        if (root.filename().string() == "build") {
            root = root.parent_path();
        }
        std::filesystem::path imgPath = root / movie.image;
        QPixmap pixmap(QString::fromStdString(imgPath.string()));
        if (!pixmap.isNull()) {
            m_imageLabel->setPixmap(pixmap.scaled(
                m_imageLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
        } else {
            m_imageLabel->setText(QStringLiteral("Image not found"));
        }
    } else {
        m_imageLabel->setText(QStringLiteral("No image"));
    }
}

void MainWindow::clearDetails() {
    m_detailTitle->setText(QStringLiteral("-"));
    m_detailYear->setText(QStringLiteral("-"));
    m_detailLength->setText(QStringLiteral("-"));
    m_detailSlogan->setText(QStringLiteral("-"));
    m_castModel->setStringList(QStringList{});
    m_imageLabel->setText(QStringLiteral("No image"));
    m_imageLabel->setPixmap(QPixmap{});
}

void MainWindow::onSortChanged(int index) {
    m_currentSortField = static_cast<core::SortField>(m_sortCombo->itemData(index).toInt());
    refreshTable();
}

void MainWindow::onSortOrderClicked() {
    m_currentSortOrder = (m_currentSortOrder == core::SortOrder::Asc) ? core::SortOrder::Desc : core::SortOrder::Asc;
    m_sortOrderBtn->setText(m_currentSortOrder == core::SortOrder::Asc ? QStringLiteral("Asc") : QStringLiteral("Desc"));
    refreshTable();
}

} // namespace ui
