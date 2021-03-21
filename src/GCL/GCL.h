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

    GCLNote(int a = 0, uint64_t b = 0, double c = 0.0, double d = 0.0) : NumQueue(a), NumMsg(b), Dur(c), Offset(d) {}
};

struct GCL {
    static uint64_t Period;

    static double IdleSlop;
    static double SendSlop;

    Link* Link_;
    std::list<GCLNote> Sch;

    static void SetPeriod(const std::vector<Message>& MSG);

    bool addMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    bool addTTMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    bool addAVBMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    void eraseMsg(const Message& msg); 
};
