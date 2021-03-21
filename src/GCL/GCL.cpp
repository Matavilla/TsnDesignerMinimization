#include "GCL.h"

uint64_t GCL::Period = 1;
double GCL::IdleSlop = 0.65;
double GCL::SendSlop = 0.35;

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

void GCL::eraseMsg(const Message& msg) {
    for (auto it = Sch.begin(); it != Sch.end();) {
        if (it->NumMsg == msg.Num) {
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

bool GCL::addAVBMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time) {
    uint64_t numFrame = msg.Size / 1500;
    if (msg.Size % 1500) {
        numFrame += 1;
    }
    double timeForTransfer = ((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth);


    bool flag = Sch.empty();
    for (auto& itTime : time) {
        uint64_t tIn = itTime.first;
        uint64_t& tOut = itTime.second;
        tOut = timeForTransfer;
        if (flag) {
            Sch.emplace_back(0, msg.Num, timeForTransfer, tIn);
            tOut += tIn;
            continue;
        }
        for (auto it = Sch.begin(); it != Sch.end();) {
            uint64_t start = it->Offset;
            uint64_t end = start + it->Dur;
            if (end <= tIn) {
                it++;
            } else {
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    GCLNote tmp(0, msg.Num, timeForTransfer, tIn);
                    tOut += tIn;
                    Sch.insert(it, std::move(tmp));
                } else {
                    int numQ = it->NumQueue;
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
    uint64_t numFrame = msg.Size / 1500;
    if (msg.Size % 1500) {
        numFrame += 1;
    }
    double timeForTransfer = ((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth);


    bool flag = Sch.empty();
    for (auto& itTime : time) {
        uint64_t tIn = itTime.first;
        uint64_t& tOut = itTime.second;
        tOut = timeForTransfer;
        if (flag) {
            Sch.emplace_back(0, msg.Num, timeForTransfer, tIn);
            tOut += tIn;
            continue;
        }
        for (auto it = Sch.begin(); it != Sch.end();) {
            uint64_t start = it->Offset;
            uint64_t end = start + it->Dur;
            if (end <= tIn) {
                it++;
            } else {
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    GCLNote tmp(0, msg.Num, timeForTransfer, tIn);
                    tOut += tIn;
                    Sch.insert(it, std::move(tmp));
                } else {
                    int numQ = it->NumQueue;
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
