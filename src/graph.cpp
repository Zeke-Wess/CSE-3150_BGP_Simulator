#include "graph.h"
#include "node.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "announcement.h"

void Graph::addNode(const int& ASN) {
    if (nodes.find(ASN) == nodes.end()) {
        nodes[ASN] = std::unique_ptr<Node>(new Node(ASN));
    }
    return;
}

void Graph::addPC(const int& provider, const int& customer) {
    addNode(provider);
    addNode(customer);
    nodes[provider]->customers.push_back(customer);
    nodes[customer]->providers.push_back(provider);
    return;
}

void Graph::addPeer(const int& first, const int& second) {
    addNode(first);
    addNode(second);
    nodes[first]->peers.push_back(second);
    nodes[second]->peers.push_back(first);
    return;
}

std::vector<int> Graph::getProviders(const int& ASN) const {
    return nodes.at(ASN)->providers;
}

std::vector<int> Graph::getCustomers(const int& ASN) const {
    return nodes.at(ASN)->customers;
}

std::vector<int> Graph::getPeers(const int& ASN) const {
    return nodes.at(ASN)->peers;
}

void Graph::checkCycles() const {
    int num_nodes = nodes.size();
    std::unordered_map<int, bool> visited;
    visited.reserve(num_nodes);
    std::unordered_map<int, bool> cur_visited;
    visited.reserve(num_nodes);

    for (const auto& [ASN, node] : nodes) {
        if(visited[ASN] == true) continue;
        else if (checkCycles_(ASN, visited, cur_visited)) {
            throw std::runtime_error("Cycle Detected");
        }
    }
    return;
}

bool Graph::checkCycles_(int n, std::unordered_map<int, bool>& visited,
                         std::unordered_map<int, bool>& cur_visited) const {
    if (cur_visited[n] == true) return true;
    if (visited[n] == true) return false;

    visited[n] = true;
    cur_visited[n] = true;

    for (int v : getCustomers(n)) {
        if (checkCycles_(v, visited, cur_visited) == true) {
            return true;
        }
    }

    cur_visited[n] = false;
    return false;
}

std::unique_ptr<Announcement> Graph::nextAnnon(
            std::unique_ptr<Announcement>& old,
            const int dest,
            const int relationship) {
    auto next = std::unique_ptr<Announcement>(new Announcement());
    next->size = old->size + 1;
    next->next_hop = old->path.front();
    next->received_from = relationship;
    next->rov_invalid = old->rov_invalid;
    next->prefix = old->prefix;
    next->path = old->path;
    next->path.push_front(dest);

    return next;

}

void Graph::sendAnnouncement(int ASN, std::unique_ptr<Announcement> annon) {
    nodes[ASN]->bgp->addToQueue(std::move(annon), nodes[ASN]->rov);
    return;
}

void Graph::flushAnnouncements(int ASN) {
    nodes[ASN]->bgp->clearQueue();
    return;
}

void Graph::flattenGraph() {
    max_rank = 0;
    int cur_rank = 0;
    std::vector<std::vector<int>> temp_ranks;
    temp_ranks.push_back(std::vector<int>());
    for (auto& [ASN, node] : nodes) {
        if (node->customers.empty()) {
            node->rank = 0;
            temp_ranks[0].push_back(ASN);
        }
    }
    do {
        temp_ranks.push_back(std::vector<int>());
        cur_rank++;
        for (int ASN : temp_ranks[cur_rank-1]) {
            for (int prov : nodes[ASN]->providers) {
                if (nodes[prov]->rank != cur_rank) {
                    nodes[prov]->rank = cur_rank;
                    temp_ranks[cur_rank].push_back(prov);
                }

            }
        }
    } while (!(temp_ranks[cur_rank].empty()));
    max_rank = cur_rank - 1;
    while (ranks.size() <= max_rank) {
        ranks.push_back(std::vector<int>());
    }
    for (auto& [ASN, node] : nodes) {
        ranks[node->rank].push_back(ASN);
    }
    return;
}

void Graph::propagateAnnons(int receiving_from) {
    switch (receiving_from) {
        case CUSTOMER:
            for (std::vector<int>& rank : ranks) {
                for (int ASN : rank) {
                    flushAnnouncements(ASN);
                }
                for (int ASN : rank) {
                    for (auto& [prefix, annon] : nodes[ASN]->bgp->rib) {
                        for (int prov : nodes[ASN]->providers) {
                            sendAnnouncement(
                                    prov,
                                    std::move(nextAnnon(annon, prov, CUSTOMER)));
                        }
                    }
                }
            }
        break;

        case PROVIDER:
            for (int i = max_rank; i>-1; i--) {
                for (int ASN : ranks[i]) {
                    flushAnnouncements(ASN);
                }
                for (int ASN : ranks[i]) {
                    for (auto& [prefix, annon] : nodes[ASN]->bgp->rib) {
                        for (int customer : nodes[ASN]->customers) {
                            sendAnnouncement(
                                customer,
                                std::move(nextAnnon(annon, customer, PROVIDER)));
                        }
                    }
                }
            }
        break;

        case PEER:
            for (auto& [ASN, node] : nodes) {
               for (auto& [prefix, annon] : node->bgp->rib) {
                    for (int peer : node->peers) {
                        sendAnnouncement(
                            peer,
                            std::move(nextAnnon(annon, peer, PEER)));
                    }
                }
            }
            for (auto& [ASN, node] : nodes) {
                flushAnnouncements(ASN);
            }

            break;
    }
}



// void Graph::propagateAnnons(std::string prefix, int receiving_from) {
//     switch (receiving_from) {
//         case CUSTOMER:
//             for (std::vector<int>& rank : ranks) {
//                 for (int ASN : rank) {
//                     flushAnnouncements(ASN);
//                     if (nodes[ASN]->bgp->rib.contains(prefix)) {
//                         auto annon = &(nodes[ASN]->bgp->rib[prefix]);
//                         for (int provider : nodes[ASN]->providers) {
//                             sendAnnouncement(
//                                 provider,
//                                 std::move(nextAnnon(*annon, provider, CUSTOMER)));
//                         }
//                     }
//                 }
//             }
//             break;
//
//         case PROVIDER:
//             for (int i = max_rank; i>-1; i--) {
//                 for (int ASN : ranks[i]) {
//                     flushAnnouncements(ASN);
//                     if (nodes[ASN]->bgp->rib.contains(prefix)) {
//                         auto annon = &(nodes[ASN]->bgp->rib[prefix]);
//                         for (int customer : nodes[ASN]->customers) {
//                             sendAnnouncement(
//                                 customer,
//                                 std::move(nextAnnon(*annon, customer, PROVIDER)));
//                         }
//                     }
//                 }
//             }
//             break;
//
//         case PEER:
//             for (auto& [ASN, node] : nodes) {
//                 if (node->bgp->rib.contains(prefix)) {
//                     auto annon = &(node->bgp->rib[prefix]);
//                     for (int peer : node->peers) {
//                         sendAnnouncement(
//                             peer,
//                             std::move(nextAnnon(*annon, peer, PEER)));
//                     }
//                 }
//             }
//             for (auto& [ASN, node] : nodes) {
//                 flushAnnouncements(ASN);
//             }
//
//             break;
//
//         default:
//             std::cout << "Bad stuff in Graph::propagateAnnons" << std::endl;
//     }
// }

void Graph::setROV(int ASN) {
    nodes[ASN]->rov = true;
}

