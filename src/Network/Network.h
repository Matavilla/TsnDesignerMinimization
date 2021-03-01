#pragma once

#include <string>
#include <vector>
#include <set>

#include "tinyxml2.h"
//#include "GCL.h"
#include "Message.h"

enum class NetElemType {
    LINK,
    ES,
    SWITCH
};

struct NetElem {
    NetElemType Type;
    int Num;

    virtual ~NetElem() = default;
};

struct Link : public NetElem {
    int Length;
    int Bandwidth;
    
    unsigned UsedCount = 0;

    NetElem* From;
    NetElem* To;

    Link() { Type = NetElemType::LINK; };

    double weight() const {
        return ((double) Length) / (UsedCount + 1);
    }
};

struct EndSystem :public NetElem {
    std::vector<Message*> Msg; // fill
    std::vector<Link*> ConnectedLinks;
    // std::vector<std::reference_wrapper<Switch>> ConnectedSwitchs;

    EndSystem() { Type = NetElemType::ES; };
};

struct Switch : public NetElem {
    std::vector<Link*> ConnectedLinks;
    // std::vector<GCL> PortGCL;
    // std::vector<std::reference_wrapper<EndSystem>> ConnectedEndSystems;
    // std::vector<std::reference_wrapper<Switch>> ConnectedSwitchs;

    Switch() { Type = NetElemType::SWITCH; };
};

struct Network {
    uint64_t MaxLength = 0;
    
    std::vector<Link> Links;
    std::vector<Switch> Switchs;
    std::vector<EndSystem> EndSystems;

//    std::vector<std::vector<std::reference_wrapper<Switch>> Graph;

    void init(tinyxml2::XMLElement* config, std::vector<Message>& msgs);

    uint64_t Length() const;
};

