#pragma once
#include <tuple>
#include <map>

class IDatabaseInterface {
public:
    virtual ~IDatabaseInterface() = default;
    virtual void saveContainer(const std::string& name, const std::tuple<std::string, double, int, int>& data) = 0;
    virtual std::tuple<std::string, double, int, int> getContainer(const std::string& name) const = 0;
    virtual size_t size() const = 0;
    virtual const std::map<std::string, std::tuple<std::string, double, int, int>>& getAll() const = 0;
};