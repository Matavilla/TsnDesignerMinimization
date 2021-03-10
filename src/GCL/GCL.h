#pragma once

#include <list>
#include <vector>
#include <utility>

#include "RoutBase.h"
#include "Message.h"
#include "Network.h"

struct GCLNote {
    int NumQueue;
    uint64_t NumMsg;
    double Dur;
    double Offset;
};

struct GCL {
    static uint64_t Period;
    Link* Link_;
    std::list<GCLNote> Sch;

    static void SetPeriod(const std::vector<Message>& MSG);

    bool addMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    void eraseMsg(const Message& msg); 
};
