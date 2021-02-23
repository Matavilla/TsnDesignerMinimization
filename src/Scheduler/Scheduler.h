#pragma once 

#include <string>
#include <vector>
#include <memory>

#include "Network.h"
#include "Message.h"
#include "RoutingDijkstra.h"


class Scheduler {
    std::unique_ptr<RoutingBase> RoutFunc;
//    std::unique_ptr<TransmissionDurBase> DurMsg;

    std::vector<Message> MSG;
    Network G;
public:
    Scheduler(const std::string& dataPath);
    
    void run();
};
