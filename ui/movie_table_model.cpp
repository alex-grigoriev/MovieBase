#include "movie_table_model.h"
#include <QString>

namespace ui {

MovieTableModel::MovieTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

void MovieTableModel::setMovies(std::vector<core::Movie> movies) {
    beginResetModel();
    m_movies = std::move(movies);
    endResetModel();
}

const core::Movie* MovieTableModel::movieAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_movies.size())) return nullptr;
    return &m_movies[row];
}

std::optional<int> MovieTableModel::movieIdAt(int row) const {
    if (const auto* m = movieAt(row)) {
        return m->id;
    }
    return std::nullopt;
}

int MovieTableModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(m_movies.size());
}

int MovieTableModel::columnCount(const QModelIndex&) const {
    return 4;
}

QVariant MovieTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant{};
    if (index.row() < 0 || index.row() >= rowCount()) return QVariant{};

    const auto& movie = m_movies[index.row()];
    switch (index.column()) {
        case 0: return QString::fromStdString(movie.title);
        case 1: return movie.year;
        case 2: return movie.length;
        case 3: return QString::fromStdString(movie.slogan);
        default: return QVariant{};
    }
}

QVariant MovieTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant{};
    switch (section) {
        case 0: return QStringLiteral("Title");
        case 1: return QStringLiteral("Year");
        case 2: return QStringLiteral("Length");
        case 3: return QStringLiteral("Slogan");
        default: return QVariant{};
    }
}

} // namespace ui
