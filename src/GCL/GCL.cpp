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
    for (auto it = Sch.begin(); it != Sch.end();) {
        if (it->NumMsg == msg.Num) {
            it = Sch.erase(it);
        } else {
            ++it;
        }
    }
}

bool addMsg(const Message& msg, std::vector<std::pair<uint64_t, uint64_t>>& time) {
    uint64_t numFrame = msg.Size / 1500;
    if (msg.Size % 1500) {
        numFrame += 1;
    }
    double timeForTransfer = ((double) numFrame * 42 + msg.Size) / (Link_->Bandwidth);


    for (auto& itTime : time) {
        uint64_t tIn = itTime.first;
        uint64_t& tOut = it.Time.second;
        tOut = timeForTransfer;
        for (auto it = Sch.begin(); it != Sch.end();) {
            uint64_t start = it->Offset;
            uint64_t end = start + it->Dur;
            if (end <= tIn) {
                it++;
            } else {
                if ((tIn + timeForTransfer) <= start) {
                    // вставляем в интервал, все круто.
                    GCLNote tmp;
                    tmp.NumQueue = 0;
                    tmp.NumMsg = msg.Num;
                    tmp.Dur = timeForTransfer;
                    tmp.Offset = tIn;
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

                    GCLNote tmp;
                    tmp.NumQueue = numQ + 1;
                    tmp.NumMsg = msg.Num;
                    tmp.Dur = timeForTransfer;
                    tmp.Offset = end;
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
