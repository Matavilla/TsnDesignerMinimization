#include <algorithm>
#include <iostream>

#include "Scheduler.h"
#include "GCL.h"
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

    GCL::SetPeriod(MSG);

    G.init(doc.FirstChildElement("Network"), MSG);
}


void Scheduler::run() {
    std::list<Paths> routes;
    for (int i = 0; i < MSG.size(); i++) {
        if (!(*RoutFunc) (G, MSG[i], routes[i])) {
            routes.erase(routes.begin() + i);
            MSG.erase(MSG.begin() + i);
            i -= 1;
            continue;
        }

        if (!assignedMsg(msg, routes[i])) {
            routes.erase(routes.begin() + i);
            MSG.erase(MSG.begin() + i);
            i -= 1;
            continue;
        }
        


    }
    printAns();
}

bool Scheduler::assignedMsg(Message& msg, Paths& r) {
    size_t countMsgs = GCL::Period / msg.T;
    for (size_t routI = 0; routI < r.Routs.size(); routI++) {
        {
            Link* link = r.Routs[routI][0];
            std::vector<std::pair<uint64_t, uint64_t>> tmp(countMsgs)
            for (size_t it = 0; it < countMsgs; it++) {
                tmp[it].first = 0;
            }
            r.Times[link] = std::move(tmp);

            EnsSystem* es = dynamic_cast<EndSystem*>(link->From);
            size_t j = 0;
            for (; j < es->ConnectedLinks.size(); j++) {
                if (sw->ConnectedLinks[j] == link) {
                    break;
                }
            }
            if (!es->PortGCL[j].addMsg(msg, r.Times[link]) {
                // все плохо
            }
        }
        for (size_t pathI = 1; pathI < r.Routs[routI].size(); pathI++) {
            Link* link = r.Routs[routI][pathI];
            if (r.Times.contains(link) {
                continue;
            }
            std::vector<std::pair<uint64_t, uint64_t>> tmp(countMsgs)
            for (size_t it = 0; it < countMsgs; it++) {
                tmp[it].first = r.Times[r.Routs[routI][pathI - 1]][it].second;
            }
            r.Times[link] = std::move(tmp);
            
            Switch* sw = dynamic_cast<Switch*>(link->From);
            size_t j = 0;
            for (; j < sw->ConnectedLinks.size(); j++) {
                if (sw->ConnectedLinks[j] == link) {
                    break;
                }
            }
            if (!sw->PortGCL[j].addMsg(msg, r.Times[link]) {
                // все плохо
            }
        }
        for (auto& it : r.Times[r.Routs[routI].back()]) {
            if (it.second > msg.MaxDur) {
            // все плохо с временем для ТТ
            }
        }

    }
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
