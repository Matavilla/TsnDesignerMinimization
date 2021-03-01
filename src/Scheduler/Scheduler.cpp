#include <algorithm>
#include <iostream>

#include "Scheduler.h"
#include "tinyxml2.h"


int comp(const Message& a, const Message& b) {
    if (a.Type == b.Type) {
        double tmp1 = a.Size / (double) a.T;
        double tmp2 = b.Size / (double) b.T;
        return tmp1 > tmp2;
    } else {
        return a.Type < b.Type;
    }
}

Scheduler::Scheduler(const std::string& dataPath) : RoutFunc(new RoutingDijkstra) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(dataPath.c_str());

    size_t i = 0;
    for (auto iter = doc.FirstChildElement("Msg"); iter != nullptr; iter = iter->NextSiblingElement("Msg")) {
        MSG.emplace_back();
        MSG.back().init(iter);
        MSG.back().Num = i;
        i++;
    }
    MaxMsg = i;
    
    std::sort(MSG.begin(), MSG.end(), comp);

    G.init(doc.FirstChildElement("Network"), MSG);
}


void Scheduler::run() {
    std::list<Paths> routes;
    for (auto i = 0; i < MSG.size(); i++) {
        if (!(*RoutFunc) (G, MSG[i], routes[i])) {
            routes.erase(routes.begin() + i);
            MSG.erase(MSG.begin() + i);
            continue;
        }
    }
    printAns();
}

void Scheduler::printAns() const {
    std::cout << "Length: " << G.Length() << std::endl;
    std::cout << "MaxLength: " << G.MaxLength << std::endl;
    std::cout << "########################################" << std::endl;
    std::cout << "AvgLength: " << ((double) G.Length()) / G.MaxLength << std::endl;

    std::cout << "########################################" << std::endl;

    std::cout << "Msg: " << MSG.size() << std::endl;
    std::cout << "MaxMsg: " << MaxMsg << std::endl;
    std::cout << "########################################" << std::endl;
    std::cout << "AvgMsg: " << ((double) MSG.size()) / MaxMsg << std::endl;
    std::cout << "########################################" << std::endl;
}
