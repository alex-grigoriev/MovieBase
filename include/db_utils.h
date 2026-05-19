#pragma once
#include "models.h"
#include "sqlite_modern_cpp.h"
#include <filesystem>
#include <string>
#include <unordered_map>

// ===== Поиск папки data рекурсивно вверх =====
std::filesystem::path find_data_dir();

// ===== Создание таблиц SQLite =====
void create_tables(sqlite::database& db);

// ===== Очистка таблиц =====
void clear_tables(sqlite::database& db);

// ===== Импорт данных в SQLite =====
void import_data(sqlite::database& db,
                 const std::vector<Movie>& movies,
                 const std::unordered_map<int, std::string>& persons);

// ===== Экспорт таблицы movies в CSV =====
void export_movies_csv(sqlite::database& db, const std::string& filename);