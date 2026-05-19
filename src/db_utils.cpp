#include "db_utils.h"
#include <fstream>
#include <algorithm>

std::filesystem::path find_data_dir() {
    std::filesystem::path dir = std::filesystem::current_path();
    while (!dir.empty()) {
        std::filesystem::path candidate = dir / "data";
        if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate)) {
            return std::filesystem::absolute(candidate);
        }
        dir = dir.parent_path();
    }
    return std::filesystem::absolute("data"); // fallback
}

void create_tables(sqlite::database& db) {
    db << "CREATE TABLE IF NOT EXISTS movies (id INTEGER PRIMARY KEY, title TEXT, year INTEGER, length INTEGER);";
    db << "CREATE TABLE IF NOT EXISTS persons (id INTEGER PRIMARY KEY, name TEXT);";
    db << "CREATE TABLE IF NOT EXISTS casting (movie_id INTEGER, person_id INTEGER, role TEXT);";
}

void clear_tables(sqlite::database& db) {
    db << "DELETE FROM movies;";
    db << "DELETE FROM persons;";
    db << "DELETE FROM casting;";
}

void import_data(sqlite::database& db,
                 const std::vector<Movie>& movies,
                 const std::unordered_map<int, std::string>& persons) {
    // Создаем map для быстрого поиска person_id по имени
    std::unordered_map<std::string, int> name_to_id;
    for (const auto& [id, name] : persons)
        name_to_id[name] = id;

    db << "BEGIN TRANSACTION;";

    for (const auto& m : movies)
        db << "INSERT INTO movies (id, title, year, length) VALUES (?, ?, ?, ?);"
           << m.id << m.title << m.year << m.length;

    for (const auto& [id, name] : persons)
        db << "INSERT INTO persons (id, name) VALUES (?, ?);" << id << name;

    for (const auto& m : movies) {
        for (const auto& c : m.cast) {
            int person_id = name_to_id[c.actor]; // быстрый поиск
            db << "INSERT INTO casting (movie_id, person_id, role) VALUES (?, ?, ?);"
               << m.id << person_id << c.role;
        }
    }

    db << "COMMIT;";
}

void export_movies_csv(sqlite::database& db, const std::string& filename) {
    std::ofstream out_csv(filename);
    out_csv << "id,title,year,length\n";

    db << "SELECT id, title, year, length FROM movies;"
       >> [&](int id, std::string title, int year, int length) {
            // Экранируем двойные кавычки
            std::replace(title.begin(), title.end(), '"', '\'');
            out_csv << id << ",\"" << title << "\"," << year << "," << length << "\n";
       };
}