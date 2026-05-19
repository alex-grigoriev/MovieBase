#include "models.h"
#include "csv_loader.h"
#include "db_utils.h"
#include <iostream>
#include "sqlite_modern_cpp.h"

int main() {
    try {
        // ===== Определяем папку data и CSV-файлы =====
        std::filesystem::path data_dir = find_data_dir();
        std::string movies_csv = (data_dir / "movies.csv").string();
        std::string persons_csv = (data_dir / "persons.csv").string();
        std::string casting_csv = (data_dir / "casting.csv").string();

        std::cout << "Current path: " << std::filesystem::current_path() << "\n";
        std::cout << "Data dir: " << data_dir << "\n";

        // ===== Загрузка CSV =====
        auto movies = load_movies(movies_csv);
        auto persons = load_persons(persons_csv);
        load_casting(casting_csv, movies, persons);

        // ===== Создаём/открываем SQLite базу =====
        sqlite::database db("movies.db");
        create_tables(db);
        clear_tables(db);
        import_data(db, movies, persons);

        // ===== Чтение фильмов =====
        std::cout << "\nMovies in database:\n";
        db << "SELECT id, title, year, length FROM movies;"
           >> [](int id, std::string title, int year, int length) {
                std::cout << id << " | " << title << " (" << year << ") " << length << " min\n";
           };

        // ===== Пример редактирования =====
        std::cout << "\nUpdating movie length for id=1 (+10 min)...\n";
        db << "UPDATE movies SET length = length + 10 WHERE id = 1;";

        // ===== Экспорт в CSV =====
        export_movies_csv(db, "exported_movies.csv");
        std::cout << "Exported updated CSV to exported_movies.csv\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }

    std::cin.get();
    return 0;
}