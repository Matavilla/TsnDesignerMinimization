#include "GCL.h"

#include <cmath>

uint64_t GCL::Period = 1;
double GCL::IdleSlopA = 0.65;
double GCL::IdleSlopB = 0.35;

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

void GCL::freeQueue(const size_t& numQ, const uint64_t& from, const uint64_t& to) {
    for (uint64_t i = from; i < to; i++) {
        SchQueue[i] &= ~(1 << numQ);
    }
}

void GCL::lockQueue(const size_t& numQ, const uint64_t& from, const uint64_t& to) {
    for (uint64_t i = from; i < to; i++) {
        SchQueue[i] |= (1 << numQ);
    }
}

int getFirstFreeQueue(const Message& msg, const uint64_t& from, const uint64_t& to) {
    for (uint64_t i = 0; i < 6; i++) {
        if (msg.Type == TypeMsg::A) {
            i = 6;
        } else if (msg.Type == TypeMsg::B) {
            i = 7;
        } 
        bool flag = false;
        for (uint64_t j = from; j < to; j++) {
            if (SchQueue[j] & (1 << i)) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            return i;
        }
    }
    return -1;
}


void GCL::eraseMsg(const Message& msg) {
    for (auto it = Sch.begin(); it != Sch.end();) {
        if (it->NumMsg == msg.Num) {
            freeQueue(it->NumQueue, it->In, it->Out);
            it = Sch.erase(it);
        } else {
            ++it;
        }
    }
}

bool GCL::addMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time) {
    if (msg.Type == TypeMsg::TT) {
        return addTTMsg(msg, time);
    } else {
        return addAVBMsg(msg, time);
    }
}

// need Network Calculus work
bool GCL::addAVBMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    uint64_t timeForTransfer = std::ceil(((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth));
    uint64_t timeGB = std::ceil(1500 / Link_->Bandwidth);

    double idleSlop = (msg.Type == TypeMsg::A) ? IdleSlopA : IdleSlopB;

    idleSlop *= Link_->Bandwidth;
    double sendSlop = Link_->Bandwidth - idleSlop;

    double credit = 0.0;
    bool flag = Sch.empty();
    for (auto& itTime : time) {
        uint64_t tIn = itTime.first;
        uint64_t& tOut = itTime.second;
        tOut = timeForTransfer;

        if (flag) {
            Sch.emplace_back(numQ, msg.Num, timeForTransfer, tIn);
            tOut += tIn;
            continue;
        }

        for (auto it = Sch.begin(); it != Sch.end();) {
            uint64_t start = it->Offset;
            uint64_t end = start + it->Dur;
            if (end <= tIn) {
                if (it->NumQueue > 5) {
                    // change credit
                    credit += (start - prevEnd) * idleSlop;
                    credit -= it->Dur * sendSlop;
                } else if (prevTime < (((int64_t) start) - timeGB)) {
                    credit += (start - timeGB - prevEnd) * idleSlop;
                }
                it++;
                prevEnd = end;
            } else {
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    GCLNote tmp(numQ, msg.Num, timeForTransfer, tIn);
                    tOut += tIn;
                    Sch.insert(it, std::move(tmp));
                } else {
                    if (it->NumQueue > 5) {
                        return false;
                    }
                    it++;
                    bool endQueue = false;
                    while (it != Sch.end() && (it->Offset - end) < timeForTransfer) {
                        if (it->NumQueue == 5) {
                            endQueue = true;
                            break;
                        }
                        end = it->Offset + it->Dur;
                        numQ = std::max(numQ, it->NumQueue);
                        it++;
                    }

                    GCLNote tmp(numQ + 1, msg.Num, timeForTransfer, end);
                    tOut += end;

                    if (it == Sch.end()) {
                        if ((Period - end) < timeForTransfer) {
                            return false;
                        }
                        Sch.push_back(std::move(tmp));
                    } else {
                        if (endQueue) {
                            // нет пустого промежутка до конца очереди
                            return false;
                        }
                        Sch.insert(it, std::move(tmp));
                    }
                }
                break;
            }
        }
    }
    return true;
}

bool GCL::addTTMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    uint64_t timeForTransfer = std::ceil(((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth));

    bool flag = Sch.empty();
    for (auto& itTime : time) {
        uint64_t tIn = itTime.first;
        uint64_t& tOut = itTime.second;
        tOut = timeForTransfer;
        if (flag) {
            tOut += tIn;
            Sch.emplace_back(0, msg.Num, tIn, tIn, tOut);
            lockQueue(0, tIn, tOut);
            continue;
        }

        for (auto it = Sch.begin(); it != Sch.end();) {
            uint64_t start = it->Offset;
            uint64_t end = it->Out;
            if (end <= tIn) {
                it++;
            } else {
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    tOut += tIn;
                    int numQ = getFirstFreeQueue(msg, tIn, tOut);
                    if (numQ == -1) {
                        return false;
                    }
                    GCLNote tmp(numQ, msg.Num, tIn, tIn, tOut);
                    lockQueue(numQ, tIn, tOut);
                    Sch.insert(it, std::move(tmp));
                } else {
                    it++;
                    while (it != Sch.end() && (it->Offset - end) < timeForTransfer) {
                        end = it->Out;
                        it++;
                    }

                    if (it == Sch.end() && (Period - end) < timeForTransfer) {
                        return false;
                    }

                    tOut += end;
                    int numQ = getFirstFreeQueue(msg, tIn, tOut);
                    if (numQ == -1) {
                        return false;
                    }
                    GCLNote tmp(numQ, msg.Num, end, tIn, tOut);
                    lockQueue(numQ, tIn, tOut);

                    if (it == Sch.end()) {
                        Sch.push_back(std::move(tmp));
                    } else {
                        Sch.insert(it, std::move(tmp));
                    }
                }
                break;
            }
        }
    }
    return true;
}
