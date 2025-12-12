#pragma once
#include <memory>
#include <vector>
#include "rib.h"

class Node {
public:
    int rank = -1;
    int ASN;
    bool rov = false;
    std::vector<int> providers;
    std::vector<int> customers;
    std::vector<int> peers;
    std::unique_ptr<BGP> bgp = std::unique_ptr<BGP>(new BGP());
    Node(int ASnum) {ASN = ASnum;}

};

