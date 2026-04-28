#include "csv_importer.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace importer {

CsvImporter::CsvImporter(data::MovieRepository& repo, QObject* parent)
    : QObject(parent), m_repo(repo) {}

std::vector<std::string> CsvImporter::splitCsvLine(const std::string& line, char delim) {
    std::vector<std::string> tokens;
    std::string field;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                in_quotes = !in_quotes;
            }
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
    return tokens;
}

std::vector<core::Movie> CsvImporter::loadMovies(const std::filesystem::path& path) {
    std::vector<core::Movie> movies;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open movies CSV: " + path.string());
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = splitCsvLine(line);
        if (tokens.size() < 4) continue;
        core::Movie m;
        try {
            m.id = std::stoi(tokens[0]);
            m.year = std::stoi(tokens[2]);
            m.length = std::stoi(tokens[3]);
        } catch (...) {
            continue;
        }
        m.title = tokens[1];
        if (tokens.size() >= 7) m.slogan = tokens[6];
        if (tokens.size() >= 8) m.image = tokens[7];
        movies.push_back(m);
    }
    return movies;
}

std::unordered_map<int, std::string> CsvImporter::loadPersons(const std::filesystem::path& path) {
    std::unordered_map<int, std::string> persons;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open persons CSV: " + path.string());
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = splitCsvLine(line);
        if (tokens.size() < 2) continue;
        try {
            int id = std::stoi(tokens[0]);
            persons[id] = tokens[1];
        } catch (...) {
            continue;
        }
    }
    return persons;
}

void CsvImporter::loadCasting(const std::filesystem::path& path,
                              std::vector<core::Movie>& movies,
                              const std::unordered_map<int, std::string>& persons) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open casting CSV: " + path.string());
    }

    std::unordered_map<int, core::Movie*> movie_map;
    for (auto& m : movies) {
        movie_map[m.id] = &m;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = splitCsvLine(line);
        if (tokens.size() < 3) continue;
        try {
            int movie_id = std::stoi(tokens[0]);
            int person_id = std::stoi(tokens[1]);
            std::string role = tokens[2];

            auto it_movie = movie_map.find(movie_id);
            auto it_person = persons.find(person_id);
            if (it_movie != movie_map.end() && it_person != persons.end()) {
                it_movie->second->cast.push_back(core::Casting{it_person->second, role});
            }
        } catch (...) {
            continue;
        }
    }
}

void CsvImporter::import(const std::filesystem::path& dir) {
    try {
        auto movies = loadMovies(dir / "movies.csv");
        emit progressChanged(10);

        auto persons = loadPersons(dir / "persons.csv");
        emit progressChanged(20);

        loadCasting(dir / "casting.csv", movies, persons);
        emit progressChanged(40);

        m_repo.beginTransaction();
        m_repo.clearTables();

        for (const auto& m : movies) {
            m_repo.addMovie(m);
        }
        emit progressChanged(60);

        for (const auto& [id, name] : persons) {
            m_repo.addPerson(id, name);
        }
        emit progressChanged(80);

        for (const auto& m : movies) {
            std::unordered_map<std::string, int> name_to_id;
            for (const auto& [id, name] : persons) {
                name_to_id[name] = id;
            }
            for (const auto& c : m.cast) {
                auto it = name_to_id.find(c.actor);
                if (it != name_to_id.end()) {
                    m_repo.addCasting(m.id, it->second, c.role);
                }
            }
        }

        m_repo.commitTransaction();
        emit progressChanged(100);
        emit finished();
    }
    catch (const std::exception& e) {
        m_repo.rollbackTransaction();
        emit error(QString::fromUtf8(e.what()));
    }
}

} // namespace importer
