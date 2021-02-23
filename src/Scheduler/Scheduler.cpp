#include <algorithm>

#include "Scheduler.h"
#include "tinyxml2.h"


int comp(const Messages& a, const Messages& b) {
    if (a.Type == b.Type) {
        double tmp1 = a.Size / (double) a.T;
        double tmp2 = b.Size / (double) b.T;
        return tmp1 > tmp2;
    } else {
        return a.Type < b.Type;
    }
}

Scheduler::Scheduler(const std::string& dataPath) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(dataPath.c_str());

    for (iter = doc.FirstChildElement("Msg"); iter != nullptr; iter = iter->NextSiblingElement("Msg")) {
        MSG.emplace_back();
        MSG.back().init(iter);
    }
    
    std::sort(MSG.begin(), MSG.end(), comp);

    G.init(doc.FirstChildElement("Network"), MSG);

    RoutFunc = new RoutingDijkstra;
}


void Scheduler::run() {
    for (auto& i : MSG) {
        RoutFunc
    }
}
