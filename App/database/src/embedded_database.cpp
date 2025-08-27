#include "embedded_database.hpp"

void EmbeddedDatabase::saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) {
    db_[name] = data;
}

std::tuple<std::string, double, int, int> EmbeddedDatabase::getContainer(const std::string& name) const {
    auto it = db_.find(name);
    if (it != db_.end()) return it->second;
    return std::tuple<std::string, double, int, int>{};
}

size_t EmbeddedDatabase::size() const {
    return db_.size();
}

const std::map<std::string, std::tuple<std::string, double, int, int>>& EmbeddedDatabase::getAll() const {
    return db_;
}

void EmbeddedDatabase::removeContainer(const std::string& name) {
    db_.erase(name);
}