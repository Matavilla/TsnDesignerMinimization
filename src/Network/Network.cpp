#include "Network.h"

using namespace tinyxml2;

void Network::init(tinyxml2::XMLElement* config, std::vector<Message>& msgs) {
    XMLElement* iter;
    {
        int tmp;
        iter = config->FirstChildElement("ESCount");
        iter->QueryIntText(&tmp);
        EndSystems.resize(tmp);

        iter = config->FirstChildElement("LCount");
        iter->QueryIntText(&tmp);
        Links.resize(tmp);

        iter = config->FirstChildElement("SWCount");
        iter->QueryIntText(&tmp);
        Switchs.resize(tmp);
    }

    iter = config->FirstChildElement("Link");
    for (size_t i = 0; iter != nullptr; i++, iter = iter->NextSiblingElement("Link")) {
        Links[i].Num = i;
        iter->FirstChildElement("Length")->QueryIntText(&Links[i].Length);
        iter->FirstChildElement("Bandwidth")->QueryIntText(&Links[i].Bandwidth);

        int tmp;
        XMLElement* iter2 = iter->FirstChildElement("FromES");
        if (iter2) {
            iter2->QueryIntText(&tmp);
            tmp -= 1;
            EndSystems[tmp].ConnectedLinks.emplace_back(&Links[i]);
            EndSystems[tmp].PortGCL[&Links[i]] = std::move(GCL(&Links[i])); 
            Links[i].From = &EndSystems[tmp];
        } else {
            iter2 = iter->FirstChildElement("FromSW");
            iter2->QueryIntText(&tmp);
            tmp -= 1;
            Switchs[tmp].ConnectedLinks.emplace_back(&Links[i]);
            Switchs[tmp].PortGCL[&Links[i]] = std::move(GCL(&Links[i]));
            Links[i].From = &Switchs[tmp];
        }

        iter2 = iter->FirstChildElement("ToES");
        if (iter2) {
            iter2->QueryIntText(&tmp);
            tmp -= 1;
            Links[i].To = &EndSystems[tmp];
        } else {
            iter2 = iter->FirstChildElement("ToSW");
            iter2->QueryIntText(&tmp);
            tmp -= 1;
            Links[i].To = &Switchs[tmp];
        }

        MaxLength += Links[i].Length;
    }

    iter = config->FirstChildElement("ES");
    for (size_t i = 0; iter != nullptr; i++, iter = iter->NextSiblingElement("ES")) {
        EndSystems[i].Num = i;
        int tmp;

        XMLElement* iter2 = iter->FirstChildElement("SenderMSG");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("SenderMSG")) {
            iter2->QueryIntText(&tmp);
            tmp -= 1;
            EndSystems[i].Msg.emplace_back(&msgs[tmp]);
            msgs[tmp].Sender = i;
        }

        iter2 = iter->FirstChildElement("ReceiverMSG");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("ReceiverMSG")) {
            iter2->QueryIntText(&tmp);
            tmp -= 1;
            msgs[tmp].Receivers.insert(i);
        }
    }

    for (size_t i = 0; i < Switchs.size(); i++) {
        Switchs[i].Num = i;
    }
}

uint64_t Network::Length() const {
    uint64_t tmp = 0;
    for (auto& i : Links) {
        if (i.UsedCount) {
            tmp += i.Length;
        }
    }
    return tmp; 
}