#pragma once

#include "models.h"
#include "movie_repository.h"
#include <vector>
#include <string_view>
#include <algorithm>
#include <ranges>

namespace service {

class MovieService {
public:
    explicit MovieService(data::MovieRepository& repo);

    [[nodiscard]] std::vector<core::Movie> getAllMovies(core::SortField field = core::SortField::Title, core::SortOrder order = core::SortOrder::Asc) const;
    [[nodiscard]] std::vector<core::Movie> searchMovies(std::string_view query, core::SortField field = core::SortField::Title, core::SortOrder order = core::SortOrder::Asc) const;
    [[nodiscard]] std::vector<core::Movie> searchAndFilter(
        std::string_view query,
        const core::MovieFilters& filters,
        core::SortField field = core::SortField::Title,
        core::SortOrder order = core::SortOrder::Asc) const;
    [[nodiscard]] std::optional<core::Movie> getMovieById(int id) const;

private:
    data::MovieRepository& m_repo;
};

} // namespace service
