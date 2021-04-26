#pragma once

#include <list>
#include <vector>
#include <utility>

#include "RoutBase.h"
#include "Message.h"
#include "Network.h"

struct Link;

struct GCLNote {
    int NumQueue;
    uint64_t NumMsg;
    int64_t Offset; // us

    int64_t In; // us
    int64_t Out; // us

    GCLNote(int a = 0, uint64_t b = 0, int64_t c = 0, int64_t t1 = 0, int64_t t2 = 0) : NumQueue(a), NumMsg(b), Offset(c), In(t1), Out(t2) {}
};

struct GCL {
    // DO NOT USE VERY BIG PERIOD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    static uint32_t Period; // ms

    static double IdleSlopA;
    static double IdleSlopB;

    Link* Link_;
    std::list<GCLNote> Sch;
    std::vector<uint8_t> SchQueue; // bits: 0 0 0 0 0 0 0 0
                                   // NumQ: 7 6 5 4 3 2 1 0

    static void SetPeriod(const std::vector<Message>& MSG);

    bool addMsg(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time);

    bool addTTMsg(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time);

    bool addAVBMsg(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time);

    bool addES(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time);

    bool checkAVB();

    void eraseMsg(const Message& msg); 

    void freeQueue(const size_t& numQ, const int64_t& from, const int64_t& to);

    void lockQueue(const size_t& numQ, const int64_t& from, const int64_t& to);

    int getFirstFreeQueue(const Message& msg, const int64_t& from, const int64_t& to);

    bool checkQueueFree(const size_t& numQ, const int64_t& from, const int64_t& to, int64_t& begin);

    bool checkQueueFree(const size_t& numQ, const int64_t& from, const int64_t& to);

    GCL(Link* l = nullptr) {
        Link_ = l;
        SchQueue.resize(Period);
    }

    GCL(const GCL& g) {
        Link_ = g.Link_;
        for (auto& i : g.Sch) {
            Sch.push_back(i);
        }
        for (auto& i : g.SchQueue) {
            SchQueue.push_back(i);
        }
    }

    GCL& operator=(const GCL& g) {
        Link_ = g.Link_;
        Sch.clear();
        SchQueue.clear();
        for (auto& i : g.Sch) {
            Sch.push_back(i);
        }
        for (auto& i : g.SchQueue) {
            SchQueue.push_back(i);
        }
    }
};
