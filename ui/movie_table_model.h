#pragma once

#include "models.h"
#include <QAbstractTableModel>
#include <vector>

namespace ui {

class MovieTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit MovieTableModel(QObject* parent = nullptr);

    void setMovies(std::vector<core::Movie> movies);
    [[nodiscard]] const core::Movie* movieAt(int row) const;
    [[nodiscard]] std::optional<int> movieIdAt(int row) const;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    std::vector<core::Movie> m_movies;
};

} // namespace ui
