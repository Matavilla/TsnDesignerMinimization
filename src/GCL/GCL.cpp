#include "GCL.h"

uint64_t GCL::Period = 1;

uint64_t gcd(uint64_t a, uint64_t b) {
    uint64_t t;
    while (b != 0) {
        t = b;
        b = a % b;
        a = t;
    }
    return a;
}

uint64_t lcm(uint64_t a, uint64_t b) {
    return a / gcd(a, b) * b;
}

void GCL::SetPeriod(const std::vector<Message>& MSG) {
    for (auto& i : MSG) {
        Period = lcm(Period, i.T);
    }
}

void eraseMsg(const Message& msg) {
    for (auto& it = Sch.begin(); it != Sch.end();) {
        if (it->NumMsg == msg.Num) {
            it = Sch.erase(it);
        } else {
            ++it;
        }
    }
}

bool addMsg(const Message& msg, std::vector<uint64_t>& time) {
    for (auto t& : time) {
        for (auto it& = Sch.begin(); it != Sch.end();) {

        }
    }
    return true;
}
