#include "Network.h"

using namespace tinyxml2;

void Network::init(tinyxml2::XMLElement* config, std::vector<Message>& msgs) {
    XMLElement* iter;
    {
        int tmp;
        iter = config->FirstChildElement("ESCount");
        iter->QueryIntText(&tmp);
        EndSystems.resize(tmp);

        iter = config->FirstChildElement("LinkCount");
        iter->QueryIntText(&tmp);
        Links.resize(tmp);

        iter = config->FirstChildElement("SwitchCount");
        iter->QueryIntText(&tmp);
        Switchs.resize(tmp);
    }

    iter = config->FirstChildElement("Link");
    for (size_t i = 0; iter != nullptr; i++, iter = iter->NextSiblingElement("Link")) {
        Links[i].Num = i;
        iter->FirstChildElement("Length")->QueryIntText(&Links[i].Length);
        iter->FirstChildElement("Bandwidth")->QueryIntText(&Links[i].Bandwidth);

        int tmp;
        XMLElement* iter2 = iter->FirstChildElement("From ES");
        if (iter2) {
            iter2->QueryIntText(&tmp);
            EndSystems[tmp].ConnectedLinks.emplace_back(&Links[i]);
            EndSystems[tmp].PortGCL[&Links[i]] = GCL(); 
            Links[i].From = &EndSystems[tmp];
        } else {
            iter2 = iter->FirstChildElement("From Switch");
            iter2->QueryIntText(&tmp);
            Switchs[tmp].ConnectedLinks.emplace_back(&Links[i]);
            Switchs[tmp].PortGCL[&Links[i]] = GCL();
            Links[i].From = &Switchs[tmp];
        }

        iter2 = iter->FirstChildElement("To ES");
        if (iter2) {
            iter2->QueryIntText(&tmp);
            Links[i].To = &EndSystems[tmp];
        } else {
            iter2 = iter->FirstChildElement("To Switch");
            iter2->QueryIntText(&tmp);
            Links[i].To = &Switchs[tmp];
        }

        MaxLength += Links[i].Length;
    }

    iter = config->FirstChildElement("ES");
    for (size_t i = 0; iter != nullptr; i++, iter = iter->NextSiblingElement("ES")) {
        EndSystems[i].Num = i;
        int tmp;

        XMLElement* iter2 = iter->FirstChildElement("Sender for MSG");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Sender for MSG")) {
            iter2->QueryIntText(&tmp);
            EndSystems[i].Msg.emplace_back(&msgs[tmp]);
            msgs[tmp].Sender = i;
        }

        iter2 = iter->FirstChildElement("Receiver for MSG");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Receiver for MSG")) {
            iter2->QueryIntText(&tmp);
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
