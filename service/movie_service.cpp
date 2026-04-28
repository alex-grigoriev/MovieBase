#include "movie_service.h"
#include <algorithm>
#include <ranges>

namespace service {

MovieService::MovieService(data::MovieRepository& repo) : m_repo(repo) {}

std::vector<core::Movie> MovieService::getAllMovies(core::SortField field, core::SortOrder order) const {
    return m_repo.getAll(field, order);
}

std::vector<core::Movie> MovieService::searchMovies(std::string_view query, core::SortField field, core::SortOrder order) const {
    return m_repo.search(query, field, order);
}

std::vector<core::Movie> MovieService::searchAndFilter(
    std::string_view query,
    const core::MovieFilters& filters,
    core::SortField field,
    core::SortOrder order) const {

    std::vector<core::Movie> movies = query.empty() ? m_repo.getAll(field, order) : m_repo.search(query, field, order);

    auto view = movies | std::views::filter([&](const core::Movie& m) {
        if (filters.min_year > 0 && m.year < filters.min_year) return false;
        if (filters.max_year > 0 && m.year > filters.max_year) return false;
        if (filters.min_length > 0 && m.length < filters.min_length) return false;
        if (filters.max_length > 0 && m.length > filters.max_length) return false;
        return true;
    });

    return std::vector<core::Movie>(view.begin(), view.end());
}

std::optional<core::Movie> MovieService::getMovieById(int id) const {
    return m_repo.getById(id);
}

} // namespace service
