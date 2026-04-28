#pragma once

#include "models.h"
#include <vector>
#include <optional>
#include <string_view>
#include <string>

class QSqlDatabase;
class QSqlQuery;

namespace data {

class MovieRepository {
public:
    explicit MovieRepository(std::string_view dbPath);
    ~MovieRepository();

    MovieRepository(const MovieRepository&) = delete;
    MovieRepository& operator=(const MovieRepository&) = delete;
    MovieRepository(MovieRepository&&) = delete;
    MovieRepository& operator=(MovieRepository&&) = delete;

    [[nodiscard]] std::vector<core::Movie> getAll(core::SortField field = core::SortField::Title, core::SortOrder order = core::SortOrder::Asc) const;
    [[nodiscard]] std::vector<core::Movie> search(std::string_view query, core::SortField field = core::SortField::Title, core::SortOrder order = core::SortOrder::Asc) const;
    [[nodiscard]] std::vector<core::Movie> filter(const core::MovieFilters& filters, core::SortField field = core::SortField::Title, core::SortOrder order = core::SortOrder::Asc) const;
    [[nodiscard]] std::optional<core::Movie> getById(int id) const;

    void addMovie(const core::Movie& m);
    void addPerson(int id, std::string_view name);
    void addCasting(int movieId, int personId, std::string_view role);

    [[nodiscard]] bool isEmpty() const;

    void clearTables();
    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

private:
    void* m_db;
    std::string m_connectionName;

    [[nodiscard]] core::Movie fetchMovieWithCast(QSqlQuery& query) const;
    [[nodiscard]] QSqlDatabase& db() const;
};

} // namespace data