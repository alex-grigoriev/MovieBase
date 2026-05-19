#include "csv_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// ===== Вспомогательная функция: разделить строку по разделителю =====
static std::vector<std::string> split_csv_line(const std::string& line, char delim = ',') {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    bool in_quotes = false;
    std::string field;

    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        }
        else if (c == delim && !in_quotes) {
            tokens.push_back(field);
            field.clear();
        }
        else {
            field += c;
        }
    }
    tokens.push_back(field);
    // Убираем лишние кавычки в начале/конце
    for (auto& t : tokens) {
        t.erase(std::remove(t.begin(), t.end(), '"'), t.end());
    }
    return tokens;
}

// ===== Загрузка фильмов =====
std::vector<Movie> load_movies(const std::string& filename) {
    std::vector<Movie> movies;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open movies CSV: " + filename);
    }

    std::string line;
    std::getline(file, line); // пропустить заголовок
    while (std::getline(file, line)) {
        auto tokens = split_csv_line(line);
        if (tokens.size() < 4) continue;
        Movie m;
        m.id = std::stoi(tokens[0]);
        m.title = tokens[1];
        m.year = std::stoi(tokens[2]);
        m.length = std::stoi(tokens[3]);
        movies.push_back(m);
    }
    return movies;
}

// ===== Загрузка персон =====
std::unordered_map<int, std::string> load_persons(const std::string& filename) {
    std::unordered_map<int, std::string> persons;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open persons CSV: " + filename);
    }

    std::string line;
    std::getline(file, line); // пропустить заголовок
    while (std::getline(file, line)) {
        auto tokens = split_csv_line(line);
        if (tokens.size() < 2) continue;
        int id = std::stoi(tokens[0]);
        std::string name = tokens[1];
        persons[id] = name;
    }
    return persons;
}

// ===== Загрузка кастинга =====
void load_casting(const std::string& filename,
                  std::vector<Movie>& movies,
                  const std::unordered_map<int, std::string>& persons) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open casting CSV: " + filename);
    }

    // Создаем map movie_id -> Movie* для быстрого поиска
    std::unordered_map<int, Movie*> movie_map;
    for (auto& m : movies) {
        movie_map[m.id] = &m;
    }

    std::string line;
    std::getline(file, line); // пропустить заголовок
    while (std::getline(file, line)) {
        auto tokens = split_csv_line(line);
        if (tokens.size() < 3) continue;
        int movie_id = std::stoi(tokens[0]);
        int person_id = std::stoi(tokens[1]);
        std::string role = tokens[2];

        auto it_movie = movie_map.find(movie_id);
        auto it_person = persons.find(person_id);
        if (it_movie != movie_map.end() && it_person != persons.end()) {
            Casting c;
            c.actor = it_person->second;
            c.role = role;
            it_movie->second->cast.push_back(c);
        }
    }
}