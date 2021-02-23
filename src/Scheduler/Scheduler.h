#pragma once 

#include <string>
#include <vector>

#include "Network.h"
#include "Message.h"
#include "RoutingDijkstra.h"


class Scheduler {
    std::unique_ptr<RoutBase> RoutFunc;
    std::unique_ptr<TransmissionDurBase> DurMsg;

    std::vector<Messages> MSG;
    Network G;
public:
    Scheduler(const std::string& dataPath);
    
    void run();
};
