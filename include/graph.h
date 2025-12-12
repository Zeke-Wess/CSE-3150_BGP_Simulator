#pragma once
#include "node.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "announcement.h"

class Graph {
public:
    int max_rank = -1;
    std::unordered_map<int, std::unique_ptr<Node>> nodes;
    std::vector<std::vector<int>> ranks;
    Graph() {}
    void addNode(const int& ASN);
    void addPC(const int& provider, const int& customer);
    void addPeer(const int& first, const int& second);
    std::vector<int> getProviders(const int& ASN) const;
    std::vector<int> getCustomers(const int& ASN) const;
    std::vector<int> getPeers(const int& ASN) const;

    void checkCycles() const;
    bool checkCycles_(int n, std::unordered_map<int, bool>& visited,
                      std::unordered_map<int, bool>& cur_visited) const;
    std::unique_ptr<Announcement> nextAnnon(std::unique_ptr<Announcement>& old,
                const int dest,
                const int relationship);
    void sendAnnouncement(int ASN, std::unique_ptr<Announcement> annon);
    void flushAnnouncements(int ASN);
    void flattenGraph();
    void propagateAnnons(int receiving_from);
    void setROV(int ASN);

};
