#pragma once

#include "RoutBase.h"


class RoutingDijkstra {
public:
    bool searchRout(const Network& G, const Message& msg, Paths& rout) override;
};
