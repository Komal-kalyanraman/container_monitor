#pragma once
#include "database_interface.hpp"
#include <map>
#include <tuple>

class EmbeddedDatabase : public IDatabaseInterface {
public:
    void saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) override;
    std::tuple<std::string, double, int, int> getContainer(const std::string& name) const override;
    size_t size() const override;
    const std::map<std::string, std::tuple<std::string, double, int, int>>& getAll() const override;
private:
    std::map<std::string, std::tuple<std::string, double, int, int>> db_;
};