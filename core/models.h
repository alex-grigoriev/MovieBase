#pragma once

#include <string>
#include <string_view>
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
    std::string slogan;
    std::string image;
    std::vector<Casting> cast;
};

struct MovieFilters {
    int min_year = 0;
    int max_year = 0;
    int min_length = 0;
    int max_length = 0;
};

enum class SortOrder { Asc, Desc };

enum class SortField { Title, Year, Length };

} // namespace core
