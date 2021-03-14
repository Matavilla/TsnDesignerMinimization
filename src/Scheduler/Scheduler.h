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

    uint64_t MaxMsg;
    std::vector<Message> MSG;
    Network G;

    bool tryFoundBypass(Link* link, Message& msg, Paths& rout, size_t deep = 0, bool flagBypass = true);

    bool limitedSearch();
public:
    Scheduler(const std::string& dataPath);
    
    void run();

    bool assignedMsg(Message& msg, Paths& rout, size_t deep = 0);

    void printAns() const
};
