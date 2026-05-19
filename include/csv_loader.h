#pragma once
#include "models.h"
#include <string>
#include <vector>
#include <unordered_map>

// ===== Загрузка фильмов из CSV =====
std::vector<Movie> load_movies(const std::string& filename);

// ===== Загрузка персон из CSV =====
std::unordered_map<int, std::string> load_persons(const std::string& filename);

// ===== Загрузка кастинга =====
void load_casting(const std::string& filename,
                  std::vector<Movie>& movies,
                  const std::unordered_map<int, std::string>& persons);