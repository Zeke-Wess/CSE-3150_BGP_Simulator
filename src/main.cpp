#include "node.h"
#include "graph.h"
#include "announcement.h"
#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <unordered_set>
#include "rib.h"


auto pv = [] (const std::vector<int> vec) {
    for (auto val : vec) {
        std::cout << val << ", ";
    }
    return ' ';
};

void printRel(Graph& G) {
    for (const auto& [ASN, node] : G.nodes) {
        std::cout << "ASN: " << ASN << " Node: " << node->ASN << std::endl;
        std::cout << "Providers: " << pv(G.getProviders(ASN)) << std::endl;
        std::cout << "Customers: " << pv(G.getCustomers(ASN)) << std::endl;
        std::cout << "Peers: " << pv(G.getPeers(ASN)) << std::endl;
        std::cout << std::endl;
    }
    return;
}

void printAnnon (Graph& G) {
    for (auto& [ASN, node] : G.nodes) {
        for (auto& [prefix, annon] : node->bgp->rib) {
            std::cout << "\nPrinting Announcement" << std::endl;
            std::cout << "size: " << annon->size << std::endl;
            std::cout << "prefix: " << annon->prefix << std::endl;
            std::cout << "rov_invalid: " << annon->rov_invalid << std::endl;
            std::cout << "next_hop: " << annon->next_hop << std::endl;
            std::cout << "received_from: ";
            if (annon->received_from == 0) std::cout << "ORIGIN";
            else if (annon->received_from == 1) std::cout << "CUSTOMER";
            else if (annon->received_from == 2) std::cout << "PEER";
            else std::cout << "PROVIDER";
            std::cout << std::endl;
            std::cout << "path: ";
            for (int dest : annon->path) {
                std::cout << dest << ", ";
            }
            std::cout << std::endl;
        }
    }
}


void testMakingGraph() {
    std::cout << "\n Testing Making Graph" << std::endl;
    auto pv = [] (const std::vector<int> vec) {
        for (auto val : vec) {
            std::cout << val << ", ";
        }
        return ' ';
    };

    Graph G = Graph();
    G.addPC(1, 5);
    G.addPC(1, 3);
    G.addPC(2, 1);
    G.addPeer(2, 5);

    for (const auto& [ASN, node] : G.nodes) {
        std::cout << "ASN: " << ASN << " Node: " << node->ASN << std::endl;
        std::cout << "Providers: " << pv(G.getProviders(ASN)) << std::endl;
        std::cout << "Customers: " << pv(G.getCustomers(ASN)) << std::endl;
        std::cout << "Peers: " << pv(G.getPeers(ASN)) << std::endl;
    }
    return;
}

void testGraphCycle() {
    std::cout << "\nTesting Graph Cycles" << std::endl;
    Graph G = Graph();
    G.addPC(1, 2);
    G.addPC(2, 3);
    G.addPC(4, 5);
    G.addPeer(3, 4);
    G.addPC(5, 9);
    G.addPC(9, 12);

    try{
        G.checkCycles();
        std::cout << "No Cycles here, which is good" << std::endl;
    }
    catch (std::runtime_error) {
        std::cout << "Caught a runtime error when I shouldn't have" << std::endl;
    }

    G.addPC(12, 5);

    try {
        G.checkCycles();
        std::cout << "Should have found a cycle, bad news" << std::endl;
    }
    catch (std::runtime_error) {
        std::cout << "Caught a runtime error, which is good" << std::endl;
    }
}

void testGraphAnnouncements() {
    std::cout << "\nTesting Announcements\n" << std::endl;

    auto printAnnon = [] (std::unique_ptr<Announcement>& annon) {
        std::cout << "\nPrinting Announcement" << std::endl;
        std::cout << "size: " << annon->size << std::endl;
        std::cout << "prefix: " << annon->prefix << std::endl;
        std::cout << "rov_invalid: " << annon->rov_invalid << std::endl;
        std::cout << "next_hop: " << annon->next_hop << std::endl;
        std::cout << "received_from: ";
        if (annon->received_from == 0) std::cout << "ORIGIN";
        else if (annon->received_from == 1) std::cout << "CUSTOMER";
        else if (annon->received_from == 2) std::cout << "PEER";
        else std::cout << "PROVIDER";
        std::cout << std::endl;
        std::cout << "path: ";
        for (int dest : annon->path) {
            std::cout << dest << ", ";
        }
        std::cout << std::endl;
    };

    Graph G = Graph();
    G.addPC(1, 2);
    G.addPC(3, 4);
    G.addPC(4, 5);
    G.addPeer(2, 5);

    auto A1 = std::unique_ptr<Announcement>(new Announcement("1/0", 1, false));
    auto A2 = G.nextAnnon(A1, 2, PROVIDER);
    auto A3 = G.nextAnnon(A2, 5, PEER);
    auto A4 = G.nextAnnon(A3, 4, CUSTOMER);

    printAnnon(A1);
    printAnnon(A2);
    printAnnon(A3);
    printAnnon(A4);

    return;
}

void testNodeRib() {
    std::cout << "\n Testing Ribs \n" << std::endl;

    auto printAnnon = [] (std::unique_ptr<Announcement>& annon) {
        std::cout << "\nPrinting Announcement" << std::endl;
        std::cout << "size: " << annon->size << std::endl;
        std::cout << "prefix: " << annon->prefix << std::endl;
        std::cout << "rov_invalid: " << annon->rov_invalid << std::endl;
        std::cout << "next_hop: " << annon->next_hop << std::endl;
        std::cout << "received_from: ";
        if (annon->received_from == 0) std::cout << "ORIGIN";
        else if (annon->received_from == 1) std::cout << "CUSTOMER";
        else if (annon->received_from == 2) std::cout << "PEER";
        else std::cout << "PROVIDER";
        std::cout << std::endl;
        std::cout << "path: ";
        for (int dest : annon->path) {
            std::cout << dest << ", ";
        }
        std::cout << std::endl;
    };

    auto printRib = [&printAnnon] (std::unique_ptr<BGP>& rib) {
        std::cout << "\nPrinting rib" << std::endl;
        for (auto& [prefix, annon] : rib->rib) {
            std::cout << "\nPrinting prefix " << prefix << std::endl;
            printAnnon(annon);
        }
    };


    Graph G = Graph();
    G.addPC(2, 1);
    G.addPC(1, 3);
    G.addPC(4, 5);
    G.addPeer(1, 4);
    G.addPeer(3, 5);
    std::unique_ptr<Announcement> seed;
    seed = std::unique_ptr<Announcement>(new Announcement("1.0.1/16", 1, false));
    G.sendAnnouncement(1, std::move(seed));
    G.flushAnnouncements(1);

    G.sendAnnouncement(2,
                    G.nextAnnon(G.nodes[1]->bgp->rib["1.0.1/16"], 2, CUSTOMER));
    G.flushAnnouncements(2);

    G.sendAnnouncement(4, G.nextAnnon(G.nodes[1]->bgp->rib["1.0.1/16"], 4, PEER));
    G.flushAnnouncements(4);

    G.sendAnnouncement(1,
                    G.nextAnnon(G.nodes[2]->bgp->rib["1.0.1/16"], 1, PROVIDER));

    G.flushAnnouncements(1);

    G.sendAnnouncement(3,
                    G.nextAnnon(G.nodes[1]->bgp->rib["1.0.1/16"], 3, PROVIDER));
    G.flushAnnouncements(3);

    G.sendAnnouncement(5,
                    G.nextAnnon(G.nodes[4]->bgp->rib["1.0.1/16"], 5, PROVIDER));
    G.flushAnnouncements(5);

    for (auto& [ASN, node] : G.nodes) {
        std::cout << "\nPrinting node " << ASN << std::endl;
        printRib(node->bgp);
    }

}

void testGraphFlattening() {
    auto printRanks = [] (std::vector<std::vector<int>>& ranks) {
        for (int i = 0; i<ranks.size(); i++) {
            std::cout << "Printing Rank " << i << ":\n" << std::endl;
            for (auto& ASN : ranks[i]) {
                std::cout << ASN << std::endl;
            }
            std::cout << std::endl;
        }
    };

    Graph G;
    G.addPC(2, 1);
    G.addPC(1, 3);
    G.addPC(4, 5);
    G.addPC(5, 6);
    G.addPeer(1, 4);
    G.addPeer(3, 5);

    G.flattenGraph();
    printRanks(G.ranks);
}

void testAnnouncementPropagation() {

    auto printAnnon = [] (std::unique_ptr<Announcement>& annon) {
        std::cout << "\nPrinting Announcement" << std::endl;
        std::cout << "size: " << annon->size << std::endl;
        std::cout << "prefix: " << annon->prefix << std::endl;
        std::cout << "rov_invalid: " << annon->rov_invalid << std::endl;
        std::cout << "next_hop: " << annon->next_hop << std::endl;
        std::cout << "received_from: ";
        if (annon->received_from == 0) std::cout << "ORIGIN";
        else if (annon->received_from == 1) std::cout << "CUSTOMER";
        else if (annon->received_from == 2) std::cout << "PEER";
        else std::cout << "PROVIDER";
        std::cout << std::endl;
        std::cout << "path: ";
        for (int dest : annon->path) {
            std::cout << dest << ", ";
        }
        std::cout << std::endl;
    };

    auto printRib = [&printAnnon] (std::unique_ptr<BGP>& rib) {
        std::cout << "\nPrinting rib of size" << rib->rib.size() << std::endl;
        for (auto& [prefix, annon] : rib->rib) {
            std::cout << "\nPrinting prefix " << prefix << std::endl;
            printAnnon(annon);
        }
    };



    Graph G;
    G.addPC(2, 1);
    G.addPC(1, 7);
    G.addPC(4, 5);
    G.addPC(5, 6);
    G.addPeer(1, 4);
    G.addPeer(7, 5);

    G.flattenGraph();
    G.sendAnnouncement(1, std::move(std::unique_ptr<Announcement>(
                       new Announcement("1.0/16", 1, false))));
    G.sendAnnouncement(5, std::move(std::unique_ptr<Announcement>(
                       new Announcement("5.5.32/16", 5, false))));
    for (int i = 1; i<4; i++) {
        G.propagateAnnons(i);
    }
    for (auto& [ASN, node] : G.nodes) {
        std::cout << "\nPrinting node " << ASN << std::endl;
        printRib(node->bgp);
    }
}


void testROV() {
    auto printAnnon = [] (std::unique_ptr<Announcement>& annon) {
        std::cout << "\nPrinting Announcement" << std::endl;
        std::cout << "size: " << annon->size << std::endl;
        std::cout << "prefix: " << annon->prefix << std::endl;
        std::cout << "rov_invalid: " << annon->rov_invalid << std::endl;
        std::cout << "next_hop: " << annon->next_hop << std::endl;
        std::cout << "received_from: ";
        if (annon->received_from == 0) std::cout << "ORIGIN";
        else if (annon->received_from == 1) std::cout << "CUSTOMER";
        else if (annon->received_from == 2) std::cout << "PEER";
        else std::cout << "PROVIDER";
        std::cout << std::endl;
        std::cout << "path: ";
        for (int dest : annon->path) {
            std::cout << dest << ", ";
        }
        std::cout << std::endl;
    };

    auto printRib = [&printAnnon] (std::unique_ptr<BGP>& rib) {
        std::cout << "\nPrinting rib of size" << rib->rib.size() << std::endl;
        for (auto& [prefix, annon] : rib->rib) {
            std::cout << "\nPrinting prefix " << prefix << std::endl;
            printAnnon(annon);
        }
    };



    Graph G;
    G.addPC(2, 1);
    G.addPC(1, 3);
    G.addPC(4, 5);
    G.addPC(5, 6);
    G.addPC(4, 6);
    G.addPeer(1, 4);
    G.addPeer(3, 5);
    G.setROV(4);

    G.flattenGraph();
    G.sendAnnouncement(1, std::move(std::unique_ptr<Announcement>(
                       new Announcement("1.0/16", 1, false))));
    G.sendAnnouncement(5, std::move(std::unique_ptr<Announcement>(
                       new Announcement("1.0/16", 5, true))));
    for (int i = 1; i<4; i++) {
        G.propagateAnnons(i);
    }
    for (auto& [ASN, node] : G.nodes) {
        std::cout << "\nPrinting node " << ASN << std::endl;
        printRib(node->bgp);
    }
}


void createGraph(Graph& G, const std::string& rel_path) {
    std::string line;
    std::ifstream rel_file (rel_path);
    std::vector<int> info;
    if (rel_file.is_open()) {
        while (getline(rel_file, line)) {
            if (line[0] == '#') continue;
            int start = 0;
            // node 1 | node 2 | relationship |
            while (line.substr(start).find('|') != std::string::npos) {
                int pos = line.substr(start).find('|');
                info.push_back(std::stoi(line.substr(start, pos)));
                start += pos+1;
            }
            if (info[2] == -1) {
                G.addPC(info[0], info[1]);
            }
            else {
                G.addPeer(info[0], info[1]);
            }
            info.clear();
        }
    }
    else {
        std::cout << "Relationship file failed to open" << std::endl;
    }
    rel_file.close();

}

std::unordered_set<std::string> seedGraph(Graph& G, std::string& annon_path) {
    std::string line;
    std::ifstream annon_file (annon_path);
    std::vector<std::string> info;
    std::unordered_set<std::string> annons;
    if (annon_file.is_open()) {
        while (getline(annon_file, line)) {
            if (line[0] == 's') continue; // ignore first line
            int start = 0;
            bool rov = false;
            std::string prefix;
            int ASN;
            while (line.substr(start).find(',') != std::string::npos) {
                int pos = line.substr(start).find(',');
                info.push_back(line.substr(start, pos));
                start += pos + 1;
            }
            if (line.substr(start).find("T") != std::string::npos)  {
                rov = true;
            }
            ASN = std::stoi(info[0]);
            prefix = info[1];
            G.sendAnnouncement(ASN, std::move(std::unique_ptr<Announcement>(
                               new Announcement(prefix, ASN, rov))));
            G.flushAnnouncements(ASN);
            annons.insert(prefix);

            info.clear();
        }
    }
    else {
        std::cout << "Anouncement file failed to open" << std::endl;
    }
    annon_file.close();
    return annons;
}

void setRovs(Graph& G, const std::string& rov_path) {
    std::string line;
    std::ifstream rov_file (rov_path);

    if (rov_file.is_open()) {
        while (getline(rov_file, line)) {
            if (line.size() <= 0) continue;
            G.setROV(std::stoi(line));
        }
    }
    else {
        std::cout << "Rov File failed to open" << std::endl;
    }
    rov_file.close();
}

void writeGraphToCsv(Graph& G) {
    std::ofstream ribs ("ribs.csv");
    std::string path;

    if (ribs.is_open()) {
        ribs << "asn,prefix,as_path\n";
        for (auto& [ASN, node] : G.nodes) {
            for (auto& [prefix, annon] : node->bgp->rib) {
                path.clear();
                path = "\"(";
                for (int step : annon->path) {
                    path += std::to_string(step);
                    path += ", ";
                }
                if (annon->path.size() != 1) {
                    path.erase(path.size()-2, 2);
                }
                else {
                    path.erase(path.size()-1, 1);
                }
                path += ")\"";
                ribs << ASN << ',' << prefix << ',' << path << std::endl;
            }
        }
    }
    else {
        std::cout << "failed to open ribs.csv" << std::endl;
    }
    ribs.close();
}


int main(int argc, char** argv) {

    //testMakingGraph();
    //testGraphCycle();
    //testGraphAnnouncements();
    //testNodeRib();
    //testGraphFlattening();
    //testAnnouncementPropagation();
    //testROV();

    Graph G;
    std::string rel_path = " ";
    std::string annon_path = " ";
    std::string rov_path = " ";

    for (int i = 0; i < argc; i++) {
        if (std::strcmp(argv[i], "--relationships") == 0) {
            rel_path = argv[i+1];
        }
        if (std::strcmp(argv[i], "--announcements") == 0) {
            annon_path = argv[i+1];
        }
        if (std::strcmp(argv[i], "--rov-asns") == 0) {
            rov_path = argv[i+1];
        }
    }

    if (rel_path == " " || annon_path == " " || rov_path == " ") {
        std::cout << "One of the paths failed to read" << std::endl;
    }

    createGraph(G, rel_path);

    try {
        G.checkCycles();
    }
    catch (std::runtime_error& e) {
        std::cout << "Caught an error" << e.what() << std::endl;
        return 1;
    }

    std::unordered_set<std::string> annons;
    annons = seedGraph(G, annon_path);
    G.flattenGraph();
//    printRel(G);

    setRovs(G, rov_path);

    for (int i=1; i<4; i++) {
        // Internally Customer, Peer, Provider are just 1, 2, 3
        // All of that just so I could make this for loop work
        G.propagateAnnons(i);
    }

    for (int cust : G.nodes[9583]->peers) {
        if (cust == 7195) {
            std::cout << "Peer found" << std::endl;
        }
    }

    for (int prov : G.nodes[45582]->providers) {
        if (prov == 9583) {
            std::cout << "Provider found" << std::endl;
        }
    }

    if (G.nodes[9583]->bgp->rib.contains("10.0.0.0/24")) {
        std::cout << "Received_from : " << G.nodes[9583]->bgp->rib["10.0.0.0/24"]->received_from << std::endl;
    } else {
        std::cout << "thing not found" << std::endl;
    }

    writeGraphToCsv(G);

    return 0;
}

