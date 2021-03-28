#include <algorithm>
#include <iostream>
#include <limits>
#include <cmath>
#include <iterator>

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
    std::vector<Paths> routes;
    for (int i = 0; i < MSG.size(); i++) {
        if (!(*RoutFunc) (G, MSG[i], routes[i])) {
            routes.erase(routes.begin() + i);
            MSG.erase(MSG.begin() + i);
            i -= 1;
            continue;
        }

        if (!assignedMsg(MSG[i], routes[i])) {
            // огр.перебор
            if (!limitedSearch(i, routes)) {
                routes.erase(routes.begin() + i);
                MSG.erase(MSG.begin() + i);
                i -= 1;
                continue;
            }
        }
        
        if (!checkTime(MSG[i], routes[i])) {
            // огр.перебор + обойти ебучие линки с буферизацией
            bool flag = false;
            for (size_t j = 1; j <= 3; j++) {
                if (tryBypassSwitchWithBuff(i, routes, j)) {
                    flag = true;
                    break;
                }
            }

            if (!flag) {
                routes.erase(routes.begin() + i);
                MSG.erase(MSG.begin() + i);
                i -= 1;
                continue;
            }
        }

        for (auto& link : routes[i].UsedLinks) {
            link->UsedCount++;
        }

    }
    printAns();
}

bool Scheduler::checkTime(Message& msg, Paths& r) {
    for (auto& p : r.Routs) {
        for (auto& it : r.Times[p.back()]) {
            if (it.second > msg.MaxDur) {
                return false;
            }
        }
    }
    return true;
}

void Scheduler::deleteMsg(Message& msg, Paths& r) {
    for (auto& link : r.UsedLinks) {
        if (dynamic_cast<EndSystem*>(link->From)) {
            dynamic_cast<EndSystem*>(link->From)->PortGCL[link].eraseMsg(msg);
        } else {
            dynamic_cast<Switch*>(link->From)->PortGCL[link].eraseMsg(msg);
        }
    }
    r.Times.clear();
}

bool Scheduler::tryBypassSwitchWithBuff(int index, std::vector<Paths>& routes, size_t subSetSize) {
    Message& msg = MSG[index];
    Paths& r = routes[index];

    size_t n = r.UsedLinks.size();
    std::vector<int> indexes(subSetSize);
    for (size_t i = 0; i < subSetSize; i++) indexes[i] = i;

    deleteMsg(msg, r);

    do {
        std::vector<int> prevLength(subSetSize);
        {
            size_t j = 0;
            for (auto& i : indexes) {
                auto tmp = r.UsedLinks.begin();
                std::advance(tmp, i);
                prevLength[j] = (*tmp)->Length;
                (*tmp)->Length = std::numeric_limits<int>::max();
                j++;
            }
        }

        Paths tmp;
        bool flag = false;
        if ((*RoutFunc) (G, msg, tmp)) {
            if (!assignedMsg(msg, tmp)) {
                if (limitedSearch(index, routes)) {
                    flag = true;
                }
            } else {
                flag = true;
            }
        }

        if (flag) {
            if (checkTime(msg, tmp)) {
                r = std::move(tmp);
                return true;
            } else {
                deleteMsg(msg, tmp);
            }
        }

        {
            size_t j = 0;
            for (auto& i : indexes) {
                auto tmp = r.UsedLinks.begin();
                std::advance(tmp, i);
                (*tmp)->Length = prevLength[j];
                j++;
            }
        }
    } while (next_combination(indexes, n - 1));

    return false;
}

bool Scheduler::tryFoundBypass(Link* link, Message& msg, Paths& r, size_t deep) {
    int prevLength = link->Length;
    link->Length = std::numeric_limits<int>::max();

    Paths tmp;

    if (!(*RoutFunc) (G, msg, tmp)) {
        std::cout << "NO, NO, IT is strange" << std::endl;
    }

    bool ans = assignedMsg(msg, tmp, deep + 1);

    if (ans) {
        r = std::move(tmp);
    }

    link->Length = prevLength;
    return ans;
}

bool Scheduler::assignedMsg(Message& msg, Paths& r, size_t deep, bool flagBypass) {
    if (deep == 5) {
        return false;
    }

    size_t countMsgs = GCL::Period / msg.T;
    for (size_t routI = 0; routI < r.Routs.size(); routI++) {
        {
            Link* link = r.Routs[routI][0];
            std::vector<std::pair<double, double>> tmp(countMsgs);
            for (size_t it = 0; it < countMsgs; it++) {
                tmp[it].first = it * msg.T;
            }
            r.Times[link] = std::move(tmp);

            EndSystem* es = dynamic_cast<EndSystem*>(link->From);

            if (!es->PortGCL[link].addMsg(msg, r.Times[link])) {
                // все плохо
                if (flagBypass) {
                    return tryFoundBypass(link, msg, r, deep);
                }
                return false;
            }
        }
        for (size_t pathI = 1; pathI < r.Routs[routI].size(); pathI++) {
            Link* link = r.Routs[routI][pathI];
            if (r.Times.contains(link)) {
                continue;
            }

            uint64_t numFrame = std::ceil(msg.Size / 1500);
            double C = ((double) (numFrame - 1) * 42 + msg.Size - 1500) / (link->Bandwidth);

            std::vector<std::pair<double, double>> tmp(countMsgs);
            for (size_t it = 0; it < countMsgs; it++) {
                tmp[it].first = r.Times[r.Routs[routI][pathI - 1]][it].second - C;
            }
            r.Times[link] = std::move(tmp);
            
            Switch* sw = dynamic_cast<Switch*>(link->From);

            if (!sw->PortGCL[link].addMsg(msg, r.Times[link])) {
                // все плохо
                deleteMsg(msg, r);

                if (flagBypass) {
                    return tryFoundBypass(link, msg, r, deep);
                }
                return false;
            }
        }
    }
    return true;
}

bool Scheduler::limitedSearch(int assignedMsgIndex, std::vector<Paths>& routes, size_t subSetSize) {
    std::vector<int> indexes(subSetSize);
    for (size_t i = 0; i < subSetSize; i++) indexes[i] = i;

    do {
        // снимаем сообщения
        for (auto& i : indexes) {
            deleteMsg(MSG[i], routes[i]); 
        }
        
        if (assignedMsg(MSG[assignedMsgIndex], routes[assignedMsgIndex])) {
            bool flag = false;
            for (auto& i : indexes) {
                if (!assignedMsg(MSG[i], routes[i], 0, false)) {
                    flag = true;
                    break;
                }

                if (!checkTime(MSG[i], routes[i])) {
                    flag = true;
                    break;
                }
            }

            if (flag) {
                for (auto& i : indexes) {
                    deleteMsg(MSG[i], routes[i]); 
                }

                deleteMsg(MSG[assignedMsgIndex], routes[assignedMsgIndex]); 
            } else {
                return true;
            }
        }

        // назначаем их
        for (auto& i : indexes) {
            if (!assignedMsg(MSG[i], routes[i])) {
                std::cout << "it's not correct limit search" << std::endl;
            }

            if (!checkTime(MSG[i], routes[i])) {
                std::cout << "it's not correct limit search in time" << std::endl;
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
