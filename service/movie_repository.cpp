#include "movie_repository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QString>
#include <QUuid>
#include <algorithm>
#include <ranges>

namespace data {

MovieRepository::MovieRepository(std::string_view dbPath) {
    QString connectionName = QUuid::createUuid().toString();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(QString::fromUtf8(dbPath.data(), static_cast<int>(dbPath.size())));
    if (!db.open()) {
        throw std::runtime_error("Failed to open database: " + db.lastError().text().toStdString());
    }

    QSqlQuery query(db);
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS movies (
            id INTEGER PRIMARY KEY,
            title TEXT,
            year INTEGER,
            length INTEGER,
            slogan TEXT,
            image TEXT
        )
    )");
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS persons (
            id INTEGER PRIMARY KEY,
            name TEXT
        )
    )");
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS casting (
            movie_id INTEGER,
            person_id INTEGER,
            role TEXT
        )
    )");

    m_connectionName = connectionName.toStdString();
    m_db = new QSqlDatabase(std::move(db));
}

MovieRepository::~MovieRepository() {
    auto* db = static_cast<QSqlDatabase*>(m_db);
    if (db->isOpen()) {
        db->close();
    }
    delete db;
    QSqlDatabase::removeDatabase(QString::fromStdString(m_connectionName));
}

QSqlDatabase& MovieRepository::db() const {
    return *static_cast<QSqlDatabase*>(m_db);
}

core::Movie MovieRepository::fetchMovieWithCast(QSqlQuery& query) const {
    core::Movie movie;
    movie.id = query.value(0).toInt();
    movie.title = query.value(1).toString().toStdString();
    movie.year = query.value(2).toInt();
    movie.length = query.value(3).toInt();
    movie.slogan = query.value(4).toString().toStdString();
    movie.image = query.value(5).toString().toStdString();

    QSqlQuery castQuery(db());
    castQuery.prepare(R"(
        SELECT p.name, c.role
        FROM casting c
        JOIN persons p ON c.person_id = p.id
        WHERE c.movie_id = ?
        ORDER BY c.rowid
    )");
    castQuery.addBindValue(movie.id);
    castQuery.exec();

    while (castQuery.next()) {
        movie.cast.push_back(core::Casting{
            castQuery.value(0).toString().toStdString(),
            castQuery.value(1).toString().toStdString()
        });
    }
    return movie;
}

static QString orderByClause(core::SortField field, core::SortOrder order) {
    QString col;
    switch (field) {
        case core::SortField::Title: col = "title"; break;
        case core::SortField::Year:  col = "year"; break;
        case core::SortField::Length: col = "length"; break;
    }
    QString ord = (order == core::SortOrder::Asc) ? "ASC" : "DESC";
    return QStringLiteral("ORDER BY %1 %2").arg(col).arg(ord);
}

std::vector<core::Movie> MovieRepository::getAll(core::SortField field, core::SortOrder order) const {
    std::vector<core::Movie> result;
    QSqlQuery query(db());
    QString sql = QStringLiteral("SELECT id, title, year, length, slogan, image FROM movies %1").arg(orderByClause(field, order));
    query.prepare(sql);
    query.exec();
    while (query.next()) {
        result.push_back(fetchMovieWithCast(query));
    }
    return result;
}

std::vector<core::Movie> MovieRepository::search(std::string_view queryStr, core::SortField field, core::SortOrder order) const {
    std::vector<core::Movie> result;
    if (queryStr.empty()) {
        return getAll(field, order);
    }

    QSqlQuery query(db());
    QString sql = QStringLiteral(R"(
        SELECT DISTINCT m.id, m.title, m.year, m.length, m.slogan, m.image
        FROM movies m
        LEFT JOIN casting c ON m.id = c.movie_id
        LEFT JOIN persons p ON c.person_id = p.id
        WHERE LOWER(m.title) LIKE '%' || LOWER(?) || '%'
           OR LOWER(p.name) LIKE '%' || LOWER(?) || '%'
           OR LOWER(c.role) LIKE '%' || LOWER(?) || '%'
        %1
    )").arg(orderByClause(field, order));
    query.prepare(sql);
    QString q = QString::fromUtf8(queryStr.data(), static_cast<int>(queryStr.size()));
    query.addBindValue(q);
    query.addBindValue(q);
    query.addBindValue(q);
    query.exec();

    while (query.next()) {
        result.push_back(fetchMovieWithCast(query));
    }
    return result;
}

std::vector<core::Movie> MovieRepository::filter(const core::MovieFilters& filters, core::SortField field, core::SortOrder order) const {
    std::vector<core::Movie> all = getAll(field, order);

    auto view = all | std::views::filter([&](const core::Movie& m) {
        if (filters.min_year > 0 && m.year < filters.min_year) return false;
        if (filters.max_year > 0 && m.year > filters.max_year) return false;
        if (filters.min_length > 0 && m.length < filters.min_length) return false;
        if (filters.max_length > 0 && m.length > filters.max_length) return false;
        return true;
    });

    return std::vector<core::Movie>(view.begin(), view.end());
}

std::optional<core::Movie> MovieRepository::getById(int id) const {
    QSqlQuery query(db());
    query.prepare("SELECT id, title, year, length, slogan, image FROM movies WHERE id = ?");
    query.addBindValue(id);
    query.exec();
    if (query.next()) {
        return fetchMovieWithCast(query);
    }
    return std::nullopt;
}

void MovieRepository::addMovie(const core::Movie& m) {
    QSqlQuery query(db());
    query.prepare("INSERT INTO movies (id, title, year, length, slogan, image) VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(m.id);
    query.addBindValue(QString::fromStdString(m.title));
    query.addBindValue(m.year);
    query.addBindValue(m.length);
    query.addBindValue(QString::fromStdString(m.slogan));
    query.addBindValue(QString::fromStdString(m.image));
    if (!query.exec()) {
        throw std::runtime_error("addMovie failed: " + query.lastError().text().toStdString());
    }
}

void MovieRepository::addPerson(int id, std::string_view name) {
    QSqlQuery query(db());
    query.prepare("INSERT INTO persons (id, name) VALUES (?, ?)");
    query.addBindValue(id);
    query.addBindValue(QString::fromUtf8(name.data(), static_cast<int>(name.size())));
    if (!query.exec()) {
        throw std::runtime_error("addPerson failed: " + query.lastError().text().toStdString());
    }
}

void MovieRepository::addCasting(int movieId, int personId, std::string_view role) {
    QSqlQuery query(db());
    query.prepare("INSERT INTO casting (movie_id, person_id, role) VALUES (?, ?, ?)");
    query.addBindValue(movieId);
    query.addBindValue(personId);
    query.addBindValue(QString::fromUtf8(role.data(), static_cast<int>(role.size())));
    if (!query.exec()) {
        throw std::runtime_error("addCasting failed: " + query.lastError().text().toStdString());
    }
}

bool MovieRepository::isEmpty() const {
    QSqlQuery query(db());
    query.exec("SELECT COUNT(*) FROM movies");
    if (query.next()) {
        return query.value(0).toInt() == 0;
    }
    return true;
}

void MovieRepository::clearTables() {
    QSqlQuery query(db());
    query.exec("DELETE FROM movies");
    query.exec("DELETE FROM persons");
    query.exec("DELETE FROM casting");
}

void MovieRepository::beginTransaction() {
    db().transaction();
}

void MovieRepository::commitTransaction() {
    db().commit();
}

void MovieRepository::rollbackTransaction() {
    db().rollback();
}

} // namespace data