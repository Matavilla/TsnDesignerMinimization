#pragma once
#include <vector>
#include <functional>

#include "tinyxml2.h"
#include "Network.h"

enum class TypeMsg {
    TT,
    A,
    B
};

struct Message {
    TypeMsg type;
    uint64_t T;
    uint64_t Size;

    uint64_t MaxDur;

    std::reference_wrapper<EndSystem> Sender;
    std::set<size_t> Receivers;

    void init(tinyxml2::XMLElement* config) {
        auto tmp = config->FirstChildElement("Type")->GetText();
        if (tmp[0] == 'T') {
            type = TypeMsg::TT;
        } else if (tmp[0] == 'A') {
            type = TypeMsg::A;
        } else {
            type = TypeMsg::B;
        }
        config->FirstChildElement("T")->QueryIntText(T);
        config->FirstChildElement("Size")->QueryIntText(Size);
        config->FirstChildElement("MaxDur")->QueryIntText(MaxDur);
    }
};
