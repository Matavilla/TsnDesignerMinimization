#include "GCL.h"

#include <cmath>
#include <iostream>

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
    Period *= 1000;
}

void GCL::freeQueue(const size_t& numQ, const int64_t& from, const int64_t& to) {
    for (int64_t i = from; i < to; i++) {
        SchQueue[i] &= ~(1 << numQ);
    }
}

void GCL::lockQueue(const size_t& numQ, const int64_t& from, const int64_t& to) {
    for (int64_t i = from; i < to; i++) {
        SchQueue[i] |= (1 << numQ);
    }
}

bool GCL::checkQueueFree(const size_t& numQ, const int64_t& from, const int64_t& to, int64_t& begin) {
    bool flag = true;
    for (int64_t j = from; j < to; j++) {
        if (SchQueue[j] & (1 << numQ)) {
            begin = j;
            flag = false;
            break;
        }
    }
    return flag;
}

bool GCL::checkQueueFree(const size_t& numQ, const int64_t& from, const int64_t& to) {
    int64_t begin;
    return checkQueueFree(numQ, from, to, begin);
}

int GCL::getFirstFreeQueue(const Message& msg, const int64_t& from, const int64_t& to) {
    for (size_t i = 0; i < 6; i++) {
        if (msg.Type == TypeMsg::A) {
            i = 6;
        } else if (msg.Type == TypeMsg::B) {
            i = 7;
        } 
        if (checkQueueFree(i, from, to)) {
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
        int64_t prevEnd = 0;
        double credit = 0.0;
        int64_t timeGB = (1542.0 / Link_->Bandwidth) * 1000.0;
        double idleSlop = (numQueue == 6) ? IdleSlopA : IdleSlopB;
        idleSlop *= (Link_->Bandwidth / 1000.0);
        double sendSlop = (Link_->Bandwidth / 1000.0) - idleSlop;
        for (auto it = Sch.begin(); it != Sch.end();) {
            int64_t start = it->Offset;
            int64_t end = it->Out;
            if (it->NumQueue < 6) {
                if (prevEnd <= (start - timeGB)) {
                    double delta = (start - timeGB - prevEnd) * idleSlop;
                    int64_t begin = 0;
                    if (checkQueueFree(numQueue, prevEnd, start - timeGB, begin)) {
                        credit += delta;
                    } else if (credit < -0.01 && (credit + delta) < -0.01) {
                        credit += delta;
                    } else if (credit < -0.01 && (credit + delta) >= -0.01) {
                        credit = 0;
                    } else {
                        credit += (begin - prevEnd) * idleSlop;
                    }
                }
            } else {
                int64_t begin = prevEnd;
                if (checkQueueFree(numQueue, prevEnd, start, begin)) {
                    credit += (start - prevEnd) * idleSlop;
                } else {
                    credit += (begin - prevEnd) * idleSlop;
                    if (credit < -0.01) {
                        credit += (start - begin) * idleSlop;
                    }
                }
                begin = it->Offset;
                if (it->NumQueue == numQueue) {
                    if (credit < -0.01) {
                        return false;
                    }
                    credit -= (it->Out - it->Offset) * sendSlop;
                } else if (checkQueueFree(numQueue, it->Offset, it->Out, begin)) {
                    credit += (it->Out - start) * idleSlop;
                } else if (begin > it->Offset) {
                    credit += (begin - it->Offset) * idleSlop;
                }
            }
            it++;
            prevEnd = end;
        }
    }
    return true;
}

bool GCL::addMsg(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time) {
    if (Link_->From->Type == NetElemType::ES) {
        return addES(msg, time);
    }
    if (msg.Type == TypeMsg::TT) {
        return addTTMsg(msg, time);
    } else {
        return addAVBMsg(msg, time);
    }
}

// need Network Calculus work
bool GCL::addAVBMsg(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    int64_t timeForTransfer = (((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth)) * 1000.0;
    int64_t timeGB = (1542.0 / Link_->Bandwidth) * 1000.0;

    double idleSlop = (msg.Type == TypeMsg::A) ? IdleSlopA : IdleSlopB;
    int numQueue = (msg.Type == TypeMsg::A) ? 6 : 7;

    idleSlop *= (Link_->Bandwidth / 1000.0);
    double sendSlop = (Link_->Bandwidth / 1000.0) - idleSlop;

    bool flag = Sch.empty();
    for (auto& itTime : time) {
        int64_t tIn = itTime.first;
        int64_t& tOut = itTime.second;
        tOut = timeForTransfer;

        double credit = 0.0;
        int64_t prevEnd = 0;
        bool flagEnd = true;
        for (auto it = Sch.begin(); it != Sch.end();) {
            int64_t start = it->Offset;
            int64_t end = it->Out;
            if (end <= tIn) {
                if (it->NumQueue < 6) {
                    if (prevEnd <= (start - timeGB)) {
                        double delta = (start - timeGB - prevEnd) * idleSlop;
                        int64_t begin = 0;
                        if (checkQueueFree(numQueue, prevEnd, start - timeGB, begin)) {
                            credit += delta;
                        } else if (credit < -0.01 && (credit + delta) < -0.01) {
                            credit += delta;
                        } else if (credit < -0.01 && (credit + delta) >= -0.01) {
                            credit = 0;
                        } else {
                            credit += (begin - prevEnd) * idleSlop;
                        }
                    }
                } else {
                    int64_t begin = prevEnd;
                    if (checkQueueFree(numQueue, prevEnd, start, begin)) {
                        credit += (start - prevEnd) * idleSlop;
                    } else {
                        credit += (begin - prevEnd) * idleSlop;
                        if (credit < -0.01) {
                            credit += (start - begin) * idleSlop;
                        }
                    }
                    begin = start;
                    if (it->NumQueue == numQueue) {
                        credit -= (end - start) * sendSlop;
                    } else if (checkQueueFree(numQueue, start, end, begin)) {
                        credit += (end - start) * idleSlop;
                    } else if (begin > start) {
                        credit += (begin - start) * idleSlop;
                    }
                }
                it++;
                prevEnd = end;
            } else {
                flagEnd = false;
                if (tIn <= start) {
                    credit += (tIn - prevEnd) * idleSlop;
                } else if (it->NumQueue < 6) {
                    if ((start - prevEnd) >= timeGB) {
                        credit += (start - prevEnd - timeGB) * idleSlop;
                    }
                } else {
                    credit += (tIn - prevEnd) * idleSlop;
                }
                int64_t waitTime = 0;
                if (credit < -0.01) {
                    waitTime = (-credit) / (idleSlop);
                }

                while (it != Sch.end()) {
                    int64_t freeTime = it->Offset - std::max(tIn, prevEnd);
                    if (it->NumQueue < 6) {
                        freeTime -= timeGB;
                    }
                    if (freeTime < 0) {
                        prevEnd = it->Out;
                        it++;
                        continue;
                    }
                    if (waitTime > 0) { // > 0
                        if (waitTime - freeTime < 0.0001) {
                            freeTime -= waitTime;
                            waitTime = 0;
                        } else {
                            waitTime -= freeTime;
                            freeTime = 0;
                        }
                    }

                    if (waitTime <= 0 ) {
                        if (it->NumQueue < 6) {
                            freeTime += timeGB;
                        }

                        if (freeTime >= timeForTransfer) {
                            int64_t tmp2;
                            tmp2 = std::max(tIn, it->Offset - freeTime);
                            tOut += tmp2;
                            if (tOut <= it->Offset) {
                                if (!checkQueueFree(numQueue, tIn, tOut)) {
                                    eraseMsg(msg);
                                    return false;
                                }
                                GCLNote tmp(numQueue, msg.Num, tmp2, tIn, tOut);
                                lockQueue(numQueue, tIn, tOut);
                                Sch.insert(it, std::move(tmp));
                                break;
                            } else {
                                tOut -= tmp2;
                            }
                        }
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

        if (flagEnd or flag) {
            tOut += tIn;
            if (!checkQueueFree(numQueue, tIn, tOut)) {
                eraseMsg(msg);
                return false;
            }
            Sch.emplace_back(numQueue, msg.Num, tIn, tIn, tOut);
            lockQueue(numQueue, tIn, tOut);
        }
        
        // check credit for all messages on this queue
        if (!checkAVB()) {
            eraseMsg(msg);
            return false;
        }
    }
    return true;
}

bool GCL::addTTMsg(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    int64_t timeForTransfer = (((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth)) * 1000.0;

    bool flag = Sch.empty();
    for (auto& itTime : time) {
        int64_t tIn = itTime.first;
        int64_t& tOut = itTime.second;
        tOut = timeForTransfer;
        /*if (flag) {
            tOut += tIn;
            Sch.emplace_back(0, msg.Num, tIn, tIn, tOut);
            lockQueue(0, tIn, tOut);
            continue;
        }*/

        bool flagEnd = true;
        for (auto it = Sch.begin(); it != Sch.end();) {
            int64_t start = it->Offset;
            int64_t end = it->Out;
            if (end < tIn) {
                it++;
            } else {
                flagEnd = false;
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
        if (flagEnd or flag) {
            if ((Period - tIn) < timeForTransfer) {
                eraseMsg(msg);
                return false;
            }
            tOut += tIn;
            Sch.emplace_back(0, msg.Num, tIn, tIn, tOut);
            lockQueue(0, tIn, tOut);
        }
    }
    if (!checkAVB()) {
        eraseMsg(msg);
        return false;
    }
    return true;
}

bool GCL::addES(const Message& msg, std::vector<std::pair<int64_t, int64_t>>& time) {
    uint64_t numFrame = std::ceil(msg.Size / 1500.0);
    int64_t timeForTransfer = (((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth)) * 1000.0;

    bool flag = Sch.empty();
    for (auto& itTime : time) {
        int64_t& tIn = itTime.first;
        int64_t& tOut = itTime.second;
        tOut = timeForTransfer;
        if (flag) {
            tOut += tIn;
            Sch.emplace_back(0, msg.Num, tIn, tIn, tOut);
            continue;
        }
        bool flagEnd = true;
        for (auto it = Sch.begin(); it != Sch.end();) {
            int64_t start = it->Offset;
            int64_t end = it->Out;
            if (end <= tIn) {
                it++;
            } else {
                flagEnd = false;
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    tOut += tIn;
                    GCLNote tmp(0, msg.Num, tIn, tIn, tOut);
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
                    GCLNote tmp(0, msg.Num, end, tIn, tOut);

                    if (it == Sch.end()) {
                        Sch.push_back(std::move(tmp));
                    } else {
                        Sch.insert(it, std::move(tmp));
                    }
                }
                break;
            }
        }
        if (flagEnd) {
            if ((Period - tIn) < timeForTransfer) {
                return false;
            }
            tOut += tIn;
            Sch.emplace_back(0, msg.Num, tIn, tIn, tOut);
        }
    }
    return true;
}
