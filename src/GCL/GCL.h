#pragma once

#include <list>
#include <vector>

#include "RoutBase.h"
#include "Message.h"

struct GCLNote {
    uint64_t NumMsg;
    uint64_t Dur;
    uint64_t Offset;
};

struct GCL {
    static uint64_t Period;
    std::list<GCLNote> Sch;

    static void SetPeriod(const std::vector<Message>& MSG);

    bool addMsg(const Message& msg, uint64_t time);

    void eraseMsg(const Message& msg); 
};
