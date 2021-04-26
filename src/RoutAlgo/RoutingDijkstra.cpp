#include <iostream>
#include <limits>
#include <algorithm>

#include "RoutingDijkstra.h"


bool RoutingDijkstra::searchRout(const Network& G, const Message& msg, Paths& rout) {
    std::vector<bool> visited(G.Switchs.size());
    std::vector<double> d(G.Switchs.size());
    std::vector<int> prev(G.Switchs.size());
    std::set<size_t> receivers(msg.Receivers);
    const EndSystem& sender = G.EndSystems[msg.Sender];


    while (!receivers.empty()) {
        bool findPath = false;
        int findReceivers = -1;
        std::vector<std::pair<int, double>> swToCheck;
        for (auto&& i : visited) i = false;
        for (auto& i : d) i = std::numeric_limits<double>::max();
        for (auto& i : prev) i = -2;
        
        for (auto& i : sender.ConnectedLinks) {
            d[i->To->Num] = (!rout.UsedLinks.contains(i)) ? i->weight() : 0;
            prev[i->To->Num] = -1; // prev = ES
        }
        
        for (size_t i = 0; i < visited.size(); i++) {
            int v = -1;
            for (size_t j = 0; j < d.size(); j++) {
                if (!visited[j] && (v == -1 or d[j] < d[v])) {
                    v = j;
                }
            }
            visited[v] = true;

            for (auto& k : G.Switchs[v].ConnectedLinks) {
                if (k->To->Type == NetElemType::ES) {
                    if (findReceivers == -1) {
                        auto it = receivers.find(k->To->Num);
                        if (it != receivers.end()) {
                            findPath = true;
                            findReceivers = k->To->Num;
                            receivers.erase(it);
                            swToCheck.emplace_back(v, (!rout.UsedLinks.contains(k)) ? k->weight() : 0);
                        }
                    } else if (k->To->Num == findReceivers) {
                        swToCheck.emplace_back(v, (!rout.UsedLinks.contains(k)) ? k->weight() : 0);
                    }
                    continue;
                }
                double len = (!rout.UsedLinks.contains(k)) ? k->weight() : 0;
                if (d[v] + len < d[k->To->Num]) {
                    d[k->To->Num] = d[v] + len;
                    prev[k->To->Num] = v;
                }
            }
        }
        if (!findPath) {
            std::cout << "DON'T FIND ROUT FOR " << msg.Num << std::endl; 
            return false;
        }

        int v = swToCheck[0].first;
        int len = swToCheck[0].second;
        for (size_t i = 1; i < swToCheck.size(); i++) {
            if ((d[v] + len) > (d[swToCheck[i].first] + swToCheck[i].second)) {
                v = swToCheck[i].first;
                len = swToCheck[i].second;
            }
        }

        std::vector<Link*> path;
        for (auto& k : G.Switchs[v].ConnectedLinks) {
            if (k->To->Type == NetElemType::ES && k->To->Num == findReceivers) {
                path.push_back(k);
                rout.UsedLinks.insert(k);
                break;
            }
        }
        int l, curNum;
        for (l = prev[v], curNum = v; l != -1 ; curNum = l, l = prev[l]) {
            for (auto& z : G.Switchs[l].ConnectedLinks) {
                if (z->To->Type == NetElemType::SWITCH && z->To->Num == curNum) {
                    path.push_back(z);
                    rout.UsedLinks.insert(z);
                    break;
                }
            }
        }
        for (auto& z : sender.ConnectedLinks) {
            if (z->To->Type == NetElemType::SWITCH && z->To->Num == curNum) {
                path.push_back(z);
                rout.UsedLinks.insert(z);
                break;
            }
        }
        std::reverse(path.begin(), path.end());

        rout.Routs.emplace_back(std::move(path));
    }
    return true;
}
