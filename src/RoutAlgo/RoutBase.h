#pragma once

#include <vector>
#include <functional>

#include "Network.h"
#include "Message.h"

using std::vector<std::vector<Link*>> Paths;

class RoutingBase {
public:
    bool operator()(const Network& G, const Message& msg, Paths& paths) {
        return this->searchRout(G, msg, paths);
    }

    virtual bool searchRout(const Network& G, const Message& msg, Paths& rout) = 0;
};
