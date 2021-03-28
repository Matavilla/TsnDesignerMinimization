#pragma once

#include <vector>
#include <set>
#include <map>
#include <utility>
#include <functional>

#include "Network.h"
#include "Message.h"

struct Link;
struct Network;

struct Paths {
    std::vector<std::vector<Link*>> Routs;
    std::set<Link*> UsedLinks;
    std::map<Link*, std::vector<std::pair<double, double>>> Times;
};

class RoutingBase {
public:
    bool operator()(const Network& G, const Message& msg, Paths& paths) {
        return this->searchRout(G, msg, paths);
    }

    virtual bool searchRout(const Network& G, const Message& msg, Paths& rout) = 0;
};
