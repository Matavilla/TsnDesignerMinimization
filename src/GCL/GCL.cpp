#include "GCL.h"

#include <cmath>

uint32_t GCL::Period = 1;
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

void GCL::freeQueue(const size_t& numQ, const double& from_, const double& to_) {
    uint64_t from = std::round(from_ * 1000);
    uint64_t to = std::round(to_ * 1000);
    for (uint64_t i = from; i < to; i++) {
        SchQueue[i] &= ~(1 << numQ);
    }
}

void GCL::lockQueue(const size_t& numQ, const double& from_, const double& to_) {
    uint64_t from = std::round(from_ * 1000);
    uint64_t to = std::round(to_ * 1000);
    for (uint64_t i = from; i < to; i++) {
        SchQueue[i] |= (1 << numQ);
    }
}

bool GCL::checkQueueFree(const size_t& numQ, const double& from_, const double& to_) {
    uint64_t from = std::round(from_ * 1000);
    uint64_t to = std::round(to_ * 1000);
    bool flag = true;
    for (uint64_t j = from; j < to; j++) {
        if (SchQueue[j] & (1 << numQ)) {
            flag = false;
            break;
        }
    }
    return flag;
}

int GCL::getFirstFreeQueue(const Message& msg, const double& from_, const double& to_) {
    for (uint64_t i = 0; i < 6; i++) {
        if (msg.Type == TypeMsg::A) {
            i = 6;
        } else if (msg.Type == TypeMsg::B) {
            i = 7;
        } 
        if (checkQueueFree(i, from_, to_)) {
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

bool GCL::checkAVB() {
    for (size_t numQueue = 6; numQueue < 8; numQueue++) {
        double prevEnd = 0.0;
        double credit = 0.0;
        double timeGB = 1542 / Link_->Bandwidth;
        double idleSlop = (numQueue == 6) ? IdleSlopA : IdleSlopB;
        idleSlop *= Link_->Bandwidth;
        double sendSlop = Link_->Bandwidth - idleSlop;
        for (auto it = Sch.begin(); it != Sch.end();) {
            double start = it->Offset;
            double end = it->Out;
            if (it->NumQueue < 6) {
                if (prevEnd <= (start - timeGB)) {
                    double delta = (start - timeGB - prevEnd) * idleSlop;
                    if (checkQueueFree(numQueue, prevEnd, start - timeGB)) {
                        credit += delta;
                    } else if (credit < 0 && (credit + delta) < 0) {
                        credit += delta;
                    } else if (credit < 0 && (credit + delta) >= 0) {
                        credit = 0;
                    }
                }
            } else {
                credit += (start - prevEnd) * idleSlop;
                if (it->NumQueue == numQueue) {
                    if (credit < 0) {
                        return false;
                    }
                    credit -= (it->Out - it->Offset) * sendSlop;
                } else if (checkQueueFree(numQueue, it->Offset, it->Out)) {
                    credit += (it->Out - start) * idleSlop;
                }
            }
            it++;
            prevEnd = end;
        }
    }
    return true;
}

bool GCL::addMsg(const Message& msg, std::vector<std::pair<double, double>>& time) {
    if (msg.Type == TypeMsg::TT) {
        return addTTMsg(msg, time);
    } else {
        return addAVBMsg(msg, time);
    }
}

// need Network Calculus work
bool GCL::addAVBMsg(const Message& msg, std::vector<std::pair<double, double>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    double timeForTransfer = ((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth);
    double timeGB = 1542 / Link_->Bandwidth;

    double idleSlop = (msg.Type == TypeMsg::A) ? IdleSlopA : IdleSlopB;
    int numQueue = (msg.Type == TypeMsg::A) ? 6 : 7;

    idleSlop *= Link_->Bandwidth;
    double sendSlop = Link_->Bandwidth - idleSlop;

    bool flag = Sch.empty();
    for (auto& itTime : time) {
        double tIn = itTime.first;
        double& tOut = itTime.second;
        tOut = timeForTransfer;

        if (flag) {
            tOut += tIn;
            if (!checkQueueFree(numQueue, tIn, tOut)) {
                eraseMsg(msg);
                return false;
            }
            Sch.emplace_back(numQueue, msg.Num, tIn, tIn, tOut);
            lockQueue(numQueue, tIn, tOut);
            continue;
        }
        
        double credit = 0.0;
        double prevEnd = 0;
        for (auto it = Sch.begin(); it != Sch.end();) {
            double start = it->Offset;
            double end = it->Out;
            if (end <= tIn) {
                if (it->NumQueue < 6) {
                    if (prevEnd <= (start - timeGB)) {
                        double delta = (start - timeGB - prevEnd) * idleSlop;
                        if (checkQueueFree(numQueue, prevEnd, start - timeGB)) {
                            credit += delta;
                        } else if (credit < 0 && (credit + delta) < 0) {
                            credit += delta;
                        } else if (credit < 0 && (credit + delta) >= 0) {
                            credit = 0;
                        }
                    }
                } else {
                    credit += (start - prevEnd) * idleSlop;
                    if (it->NumQueue == numQueue) {
                        credit -= (end - start) * sendSlop;
                    } else if (checkQueueFree(numQueue, start, end)) {
                        credit += (end - start) * idleSlop;
                    }
                }
                it++;
                prevEnd = end;
            } else {
                credit += (tIn - prevEnd) * idleSlop;
                double waitTime = 0;
                if (credit < 0) {
                    waitTime = (-credit) / (idleSlop);
                }

                if (it->NumQueue == numQueue) {
                    eraseMsg(msg);
                    return false;
                }
                while (it != Sch.end()) {
                    double freeTime = it->Offset - prevEnd;
                    if (it->NumQueue < 6) {
                        freeTime -= timeGB;
                        if (freeTime < 0) {
                            freeTime = 0;
                        }
                    }
                    if (waitTime > 0.0001) { // > 0
                        if (waitTime - freeTime < 0.0001) {
                            freeTime -= waitTime;
                            waitTime = 0;
                        } else {
                            waitTime -= freeTime;
                            freeTime = 0;
                        }
                    }

                    if (waitTime < 0.0001 && it->NumQueue < 6) {
                        freeTime += timeGB;
                    }
                    if (freeTime >= timeForTransfer) {
                        tOut += it->Offset - freeTime;
                        if (!checkQueueFree(numQueue, tIn, tOut)) {
                            eraseMsg(msg);
                            return false;
                        }
                        GCLNote tmp(numQueue, msg.Num, it->Offset - freeTime, tIn, tOut);
                        lockQueue(numQueue, tIn, tOut);
                        Sch.insert(it, std::move(tmp));
                        break;
                    }
                    prevEnd = it->Out;
                    it++;
                }

                if (it == Sch.end()) {
                    if ((Period - prevEnd) < (timeForTransfer + waitTime)) {
                        eraseMsg(msg);
                        return false;
                    } else {
                        tOut += prevEnd + waitTime;
                        if (!checkQueueFree(numQueue, tIn, tOut)) {
                            eraseMsg(msg);
                            return false;
                        }
                        GCLNote tmp(numQueue, msg.Num, prevEnd + waitTime, tIn, tOut);
                        lockQueue(numQueue, tIn, tOut);
                        Sch.push_back(std::move(tmp));
                    }
                }
                break;
            }
        }

        // check credit for all messages on this queue
        if (!checkAVB()) {
            eraseMsg(msg);
            return false;
        }
    }
    return true;
}

bool GCL::addTTMsg(const Message& msg, std::vector<std::pair<double, double>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    double timeForTransfer = ((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth);

    bool flag = Sch.empty();
    for (auto& itTime : time) {
        double tIn = itTime.first;
        double& tOut = itTime.second;
        tOut = timeForTransfer;
        if (flag) {
            tOut += tIn;
            Sch.emplace_back(0, msg.Num, tIn, tIn, tOut);
            lockQueue(0, tIn, tOut);
            continue;
        }

        for (auto it = Sch.begin(); it != Sch.end();) {
            double start = it->Offset;
            double end = it->Out;
            if (end <= tIn) {
                it++;
            } else {
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    tOut += tIn;
                    int numQ = getFirstFreeQueue(msg, tIn, tOut);
                    if (numQ == -1) {
                        eraseMsg(msg);
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
                        eraseMsg(msg);
                        return false;
                    }

                    tOut += end;
                    int numQ = getFirstFreeQueue(msg, tIn, tOut);
                    if (numQ == -1) {
                        eraseMsg(msg);
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
    if (!checkAVB()) {
        eraseMsg(msg);
        return false;
    }
    return true;
}
