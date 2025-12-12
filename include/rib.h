#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "announcement.h"

class BGP {
public:
    std::unordered_map<std::string, std::unique_ptr<Announcement>> rib;
    std::unordered_map<std::string,
                       std::vector<std::unique_ptr<Announcement>>> queue;
    void addToQueue(std::unique_ptr<Announcement> to_add, const bool ROV);
    void clearQueue();


};
