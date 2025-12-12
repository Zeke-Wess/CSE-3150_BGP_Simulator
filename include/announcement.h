#pragma once
#include <string>
#include <deque>

const int ORIGIN = 0;
const int CUSTOMER = 1;
const int PEER = 2;
const int PROVIDER = 3;

class Announcement {
public:
    int size;
    int next_hop;
    int received_from;
    bool rov_invalid;
    std::string prefix;
    std::deque<int> path;

    Announcement() {size = 1;}

    Announcement(std::string pre,
        int ASN,
        bool ROV)
    {
        size = 1;
        next_hop = ASN;
        received_from = ORIGIN;
        rov_invalid = ROV;
        prefix = pre;
        path.push_front(ASN);
    }

};
