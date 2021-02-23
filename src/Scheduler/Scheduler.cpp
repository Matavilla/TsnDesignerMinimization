#include <algorithm>

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
    
    std::sort(MSG.begin(), MSG.end(), comp);

    G.init(doc.FirstChildElement("Network"), MSG);

}


void Scheduler::run() {
    for (auto& i : MSG) {
//        RoutFunc
    }
}
