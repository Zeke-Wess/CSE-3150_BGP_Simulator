#include "rib.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "announcement.h"
#include <iostream>

void BGP::addToQueue(std::unique_ptr<Announcement> to_add, const bool ROV) {
    std::string prefix = to_add->prefix;
    if (ROV && to_add->rov_invalid) {
            return;
    }
    queue[prefix].push_back(std::move(to_add));
    return;
}

void BGP::clearQueue() {
    if (queue.empty()) return;
    for (auto& [prefix, annons] : queue) {
        int min_size = -1;
        int min_next_hop = -1;
        int min_rel = 4;
        //Relationships are just ints 0, 1, 2, 3
        std::unique_ptr<Announcement>* cur_best = nullptr;
        for (int i=0; i<annons.size() ; i++) {
            if (annons[i]->received_from < min_rel) {
                min_rel = annons[i]->received_from;
                min_size = annons[i]->size;
                min_next_hop = annons[i]->next_hop;
                cur_best = &(annons[i]);
            }
            else if (annons[i]->received_from == min_rel &&
                     annons[i]->size < min_size) {
                min_size = annons[i]->size;
                min_next_hop = annons[i]->next_hop;
                cur_best = &(annons[i]);
            }
            else if(annons[i]->size == min_size &&
                annons[i]->received_from == min_rel &&
                annons[i]->next_hop < min_next_hop) {
                min_next_hop = annons[i]->next_hop;
                cur_best = &(annons[i]);
            }

        }
        if (rib.contains(prefix)) {
            auto* champ = &(rib[prefix]);
            if ((*champ)->received_from < min_rel) {
                continue;
            } else if ((*champ)->received_from == min_rel &&
                        (*champ)->size < min_size) {
                continue;
            } else if ((*champ)->received_from == min_rel &&
                        (*champ)->size == min_size &&
                    (*champ)->next_hop < min_next_hop) {
                continue;
            }
        }
        rib[(*cur_best)->prefix] = std::move(*cur_best);
    }
    queue.clear();
    return;
}
