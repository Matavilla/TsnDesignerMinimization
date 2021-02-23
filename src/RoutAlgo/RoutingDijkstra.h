#pragma once

#include "RoutBase.h"


class RoutingDijkstra : public RoutingBase {
public:
    bool searchRout(const Network& G, const Message& msg, Paths& rout) override;
};
