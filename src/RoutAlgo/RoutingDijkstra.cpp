#include <iostream>
#include <set>
#include <limits>
#include <algorithm>

#include "RoutingDijkstra.h"


bool RoutingDijkstra::searchRout(const Network& G, const Message& msg, Paths& rout) {
    std::vector<bool> visited(G.Switchs.size());
    std::vector<double> d(G.Switchs.size());
    std::vector<unsigned> prev(G.Switchs.size());
    std::set<unsigned> usedLinks; // use Length = 0 in this Links
    std::set<size_t> receivers(msg.Receivers);

    while (!receivers.empty()) {
        bool findPath = false;
        for (auto&& i : visited) i = false;
        for (auto& i : d) i = std::numeric_limits<double>::max();
        for (auto& i : prev) i = -2;
        
        for (auto& i : msg.Sender->ConnectedLinks) {
            d[i->To->Num] = (!usedLinks.contains(i->Num)) ? i->weight() : 0;
            prev[i->To->Num] = -1; // prev = ES
        }
        
        for (size_t i = 0; i < visited.size(); i++) {
            size_t v = -1;
            for (size_t j = 0; j < d.size(); j++) {
                if (!visited[j] && (v == -1 or d[j] < d[v])) {
                    v = j;
                }
            }
            visited[v] = true;

            for (auto& k : G.Switchs[v].ConnectedLinks) {
                if (k->To->Type == NetElemType::ES) {
                    auto it = receivers.find(k->To->Num);
                    if (it != receivers.end()) {
                        findPath = true;
                        receivers.erase(it);
                        std::vector<Link*> path;

                        path.push_back(k);
                        size_t l, curNum;
                        for (l = prev[v], curNum = v; l != -1 ; curNum = v, l = prev[l]) {
                            for (auto& z : G.Switchs[l].ConnectedLinks) {
                                if (z->To->Num == curNum) {
                                    path.push_back(z);
                                    usedLinks.insert(z->Num);
                                    break;
                                }
                            }
                        }
                        for (auto& z : msg.Sender->ConnectedLinks) {
                            if (z->To->Num == curNum) {
                                path.push_back(z);
                                usedLinks.insert(z->Num);
                                break;
                            }
                        }
                        std::reverse(path.begin(), path.end());

                        rout.emplace_back(std::move(path));
                        break;
                    }
                    continue;
                }
                len = (!usedLinks.contains(k->Num)) ? k->weight() : 0;
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
    return true;
}
