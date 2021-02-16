#pragma once
#include <vector>
#include <string>
#include <functional>

#include "tinyxml2.h"
#include "GCL.h"
#include "Message.h"

struct Link {
    int Num;
    int Length;
    int Bandwidth;
};

struct EndSystem {
    int Num;

    std::vector<std::reference_wrapper<Message>> Msg; // fill
    std::vector<std::reference_wrapper<Link>> ConnectedLinks;
    std::vector<std::reference_wrapper<Switch>> ConnectedSwitchs;
};

struct Switch {
    int Num;

    std::vector<std::reference_wrapper<Link>> ConnectedLinks;
    std::vector<GCL> PortGCL;
    std::vector<std::reference_wrapper<EndSystem>> ConnectedEndSystems;
    std::vector<std::reference_wrapper<Switch>> ConnectedSwitchs;
};

struct Network {
    uint64_t MaxLength = 0;
    
    std::set<size_t> UnusedLinks;

    std::vector<Link> Links;
    std::vector<Switch> Switchs;
    std::vector<EndSystem> EndSystems;

//    std::vector<std::vector<std::reference_wrapper<Switch>> Graph;

    void init(tinyxml2::XMLElement* config, std::vector<Message>& msgs);

    double AvgLength() const;
};

