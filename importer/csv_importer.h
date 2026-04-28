#pragma once

#include "models.h"
#include "movie_repository.h"
#include <QObject>
#include <QString>
#include <filesystem>

namespace importer {

class CsvImporter : public QObject {
    Q_OBJECT
public:
    explicit CsvImporter(data::MovieRepository& repo, QObject* parent = nullptr);

    void import(const std::filesystem::path& dir);

signals:
    void progressChanged(int percent);
    void finished();
    void error(QString message);

private:
    data::MovieRepository& m_repo;

    [[nodiscard]] std::vector<core::Movie> loadMovies(const std::filesystem::path& path);
    [[nodiscard]] std::unordered_map<int, std::string> loadPersons(const std::filesystem::path& path);
    void loadCasting(const std::filesystem::path& path,
                     std::vector<core::Movie>& movies,
                     const std::unordered_map<int, std::string>& persons);

    [[nodiscard]] static std::vector<std::string> splitCsvLine(const std::string& line, char delim = ',');
};

} // namespace importer
