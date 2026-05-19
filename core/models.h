#pragma once

#include <string>
#include <vector>
#include <optional>

namespace core {

struct Casting {
    std::string actor;
    std::string role;
};

struct Movie {
    int id{};
    std::string title;
    int year{};
    int length{};
    std::string poster_path;
    std::string slogan;
    std::string image;
    std::vector<Casting> cast;
};

enum class SortField {
    Title,
    Year,
    Length
};

enum class SortOrder {
    Asc,
    Desc
};

struct MovieFilters {
    int min_year{};
    int max_year{};
    int min_length{};
    int max_length{};
};

} // namespace core