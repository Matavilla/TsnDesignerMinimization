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
    uint64_t Offset;

    uint64_t In;
    uint64_t Out

    GCLNote(int a = 0, uint64_t b = 0, uint64_t c = 0, uint64_t t1 = 0, uint64_t t2 = 0) : NumQueue(a), NumMsg(b), Offset(c), In(t1), Out(t2) {}
};

struct GCL {
    static uint64_t Period;

    static double IdleSlopA;
    static double IdleSlopB;

    Link* Link_;
    std::list<GCLNote> Sch;
    std::vector<uint8_t> SchQueue; // bits: 0 0 0 0 0 0 0 0
                                   // NumQ: 7 6 5 4 3 2 1 0

    static void SetPeriod(const std::vector<Message>& MSG);

    bool addMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    bool addTTMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    bool addAVBMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time);

    void eraseMsg(const Message& msg); 

    void freeQueue(const size_t& numQ, const uint64_t& from, const uint64_t& to);

    void lockQueue(const size_t& numQ, const uint64_t& from, const uint64_t& to);

    int getFirstFreeQueue(const Message& msg, const uint64_t& from, const uint64_t& to);

    GCL() {
        Link_ = nullptr;
        SchQueue.resize(Period);
    }
};
