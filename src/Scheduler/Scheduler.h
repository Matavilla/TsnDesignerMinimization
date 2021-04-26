#pragma once 

#include <string>
#include <vector>
#include <memory>

#include "Network.h"
#include "Message.h"
#include "RoutingDijkstra.h"


class Scheduler {
    std::unique_ptr<RoutingBase> RoutFunc;

    uint64_t MaxMsg;
    std::vector<Message> MSG;
    Network G;

    bool tryFoundBypass(Link* link, Message& msg, Paths& rout, size_t deep);

    bool limitedSearch(int index, std::vector<Paths>& r, size_t subSetSize = 2);

    bool tryBypassSwitchWithBuff(int index, std::vector<Paths>& r, size_t subSetSize = 2);

    bool checkTime(Message& msg, Paths& r);

    void deleteMsg(Message& msg, Paths& r);

    bool assignedMsg(Message& msg, Paths& rout, size_t deep = 0, bool flagBypass = true);

    void savePath(std::map<Link*,GCL>& zipGCL, std::map<Link*,std::vector<std::pair<int64_t,int64_t>>>& zipTime, Paths& r);

    void loadPath(std::map<Link*,GCL>& zipGCL, std::map<Link*,std::vector<std::pair<int64_t,int64_t>>>& zipTime, Paths& r);
public:
    Scheduler(const std::string& dataPath);
    
    void run();

    void printAns() const;
};
