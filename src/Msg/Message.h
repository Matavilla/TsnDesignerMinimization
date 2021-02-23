#pragma once

#include <set>

#include "tinyxml2.h"
#include "Network.h"

enum class TypeMsg {
    TT,
    A,
    B
};

struct Message {
    TypeMsg Type;
    uint64_t T;
    uint64_t Size;

    uint64_t MaxDur;

    EndSystem* Sender;
    std::set<size_t> Receivers;

    void init(tinyxml2::XMLElement* config) {
        auto tmp = config->FirstChildElement("Type")->GetText();
        if (tmp[0] == 'T') {
            Type = TypeMsg::TT;
        } else if (tmp[0] == 'A') {
            Type = TypeMsg::A;
        } else {
            Type = TypeMsg::B;
        }
        config->FirstChildElement("T")->QueryIntText(T);
        config->FirstChildElement("Size")->QueryIntText(Size);
        config->FirstChildElement("MaxDur")->QueryIntText(MaxDur);
    }
};
