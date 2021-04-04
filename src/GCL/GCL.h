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
    double Offset;

    double In;
    double Out;

    GCLNote(int a = 0, uint64_t b = 0, double c = 0, double t1 = 0, double t2 = 0) : NumQueue(a), NumMsg(b), Offset(c), In(t1), Out(t2) {}
};

struct GCL {
    // DO NOT USE VERY BIG PERIOD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    static uint32_t Period;

    static double IdleSlopA;
    static double IdleSlopB;

    Link* Link_;
    std::list<GCLNote> Sch;
    std::vector<uint8_t> SchQueue; // bits: 0 0 0 0 0 0 0 0
                                   // NumQ: 7 6 5 4 3 2 1 0

    static void SetPeriod(const std::vector<Message>& MSG);

    bool addMsg(const Message& msg, std::vector<std::pair<double, double>>& time);

    bool addTTMsg(const Message& msg, std::vector<std::pair<double, double>>& time);

    bool addAVBMsg(const Message& msg, std::vector<std::pair<double, double>>& time);

    bool addES(const Message& msg, std::vector<std::pair<double, double>>& time);

    bool checkAVB();

    void eraseMsg(const Message& msg); 

    void freeQueue(const size_t& numQ, const double& from, const double& to);

    void lockQueue(const size_t& numQ, const double& from, const double& to);

    int getFirstFreeQueue(const Message& msg, const double& from, const double& to);

    bool checkQueueFree(const size_t& numQ, const double& from, const double& to);

    GCL(Link* l = nullptr) {
        Link_ = l;
        SchQueue.resize(Period * 1000);
    }
};
