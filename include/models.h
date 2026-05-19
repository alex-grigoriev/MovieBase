#pragma once
#include <string>
#include <vector>

// ===== Структура для кастинга =====
struct Casting {
    std::string actor;  // имя актёра
    std::string role;   // роль в фильме
};

// ===== Структура для фильма =====
struct Movie {
    int id = 0;                 // уникальный идентификатор
    std::string title;           // название фильма
    int year = 0;                // год выпуска
    int length = 0;              // длительность в минутах
    std::vector<Casting> cast;   // список актёров с ролями
};

// ===== Структура для персона =====
struct Person {
    int id = 0;              // уникальный идентификатор
    std::string name;        // имя
};