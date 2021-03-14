#include <algorithm>
#include <iostream>
#include <limits>

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
            // огр.перебор
            

            routes.erase(routes.begin() + i);
            MSG.erase(MSG.begin() + i);
            i -= 1;
            continue;
        }
        
        for (auto& p : routes[i].Routs) {
            for (auto& it : routes[i].Times[p.back()]) {
                if (it.second > msg.MaxDur) {
                // все плохо с временем для ТТ
                // огр.перебор + обойти ебучие линки с буферизацией
                }
            }
        }

        for (auto& link : routes[i].UsedLinks) {
            link->UsedCount++;
        }

    }
    printAns();
}

bool Scheduler::tryFoundBypass(Link* link, Message& msg, Paths& r, size_t deep) {
    int prevLength = link->Length;
    link->Length = std::numeric_limits<int>::max();
    for (auto& it : r.UsedLinks) {
        if (dynamic_cast<EndSystem*>(link->From)) {
            dynamic_cast<EndSystem*>(link->From)->PortGCL[link].eraseMsg(msg);
        } else {
            dynamic_cast<Switch*>(link->From)->PortGCL[link].eraseMsg(msg);
        }
    }

    r.Routs.clear();
    r.UsedLinks.clear();
    r.Times.clear();

    if (!(*RoutFunc) (G, msg, r)) {
        std::cout << "NO, NO, IT is strange" << std::endl;
    }

    bool ans = assignedMsg(msg, r, deep + 1);
    link->Length = prevLength;
    return ans;
}

bool Scheduler::assignedMsg(Message& msg, Paths& r, size_t deep, bool flagBypass) {
    if (deep = 5) {
        return false;
    }

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

            if (!es->PortGCL[link].addMsg(msg, r.Times[link]) {
                // все плохо
                if (flagBypass) {
                    return tryFoundBypass(link, msg, r, deep);
                }
                return false;
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

            if (!sw->PortGCL[link].addMsg(msg, r.Times[link]) {
                // все плохо
                if (flagBypass) {
                    return tryFoundBypass(link, msg, r, deep);
                }
                return false;
            }
        }
    }
    return true;
}

bool next_combination(std::vector<int>& a, int n) {
    int k = a.size();
    for (int i = k - 1; i >= 0; --i) {
        if (a[i] < n - k + i + 1) {
            ++a[i];
            for (int j = i + 1; j < k; ++j) {
                a[j] = a[j - 1] + 1;
            }
            return true;
        }
    }
    return false;
}

bool Scheduler::limitedSearch(int assignedMsgIndex, std::list<Paths>& routes, size_t subSetSize = 2) {
    std::vector<size_t> indexes(subSetSize);
    for (size_t i = 0; i < subSetSize; i++) indexes[i] = i;
    do {
        // снимаем сообщения
        for (auto& i : indexes) {
            routes[i].Times.clear();
            for (auto& link : routes[i].UsedLinks) {
                if (dynamic_cast<EndSystem*>(link->From)) {
                    dynamic_cast<EndSystem*>(link->From)->PortGCL[link].eraseMsg(MSG[i]);
                } else {
                    dynamic_cast<Switch*>(link->From)->PortGCL[link].eraseMsg(MSG[i]);
                }
            }
        }
        
        if (assignedMsg(MSG[assignedMsgIndex], routes[assignedMsgIndex])) {
            bool flag = false;
            for (auto& i : indexes) {
                if (!assignedMsg(MSG[i], routes[i], 0, false)) {
                    flag = true;
                    break;
                }

                for (auto& p : routes[i].Routs) {
                    for (auto& it : routes[i].Times[p.back()]) {
                        if (it.second > MSG[i].MaxDur) {
                            flag = true;
                            break;
                        }
                    }
                }

                if (flag) {
                    break;
                }
            }

            if (flag) {
                for (auto& i : indexes) {
                    routes[i].Times.clear();
                    for (auto& link : routes[i].UsedLinks) {
                        if (dynamic_cast<EndSystem*>(link->From)) {
                            dynamic_cast<EndSystem*>(link->From)->PortGCL[link].eraseMsg(MSG[i]);
                        } else {
                            dynamic_cast<Switch*>(link->From)->PortGCL[link].eraseMsg(MSG[i]);
                        }
                    }
                }
            } else {
                return true;
            }
        }

        // назначаем их
        for (auto& i : indexes) {
            if (!assignedMsg(MSG[i], routes[i])) {
                std::cout << "it's not correct limit search" << std::endl;
            }

            for (auto& p : routes[i].Routs) {
                for (auto& it : routes[i].Times[p.back()]) {
                    if (it.second > MSG[i].MaxDur) {
                        std::cout << "it's not correct limit search in time" << std::endl;
                    }
                }
        }

    } while (next_combination(indexes, assignedMsgIndex - 1));

    return false;
};

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
