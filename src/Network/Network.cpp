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
        MaxLength += Links[i].Length;
    }

    iter = config->FirstChildElement("ES");
    for (size_t i = 0; iter != nullptr; i++, iter = iter->NextSiblingElement("ES")) {
        EndSystems[i].Num = i;
        int tmp;

        XMLElement* iter2 = iter->FirstChildElement("Connected with Link");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Connected with Link")) {
            iter2->QueryIntText(&tmp);
            EndSystems[i].ConnectedLinks.emplace_back(Links[tmp]);
        }

        iter2 = iter->FirstChildElement("Connected with Switch");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Connected with Switch")) {
            iter2->QueryIntText(&tmp);
            EndSystems[i].ConnectedSwitchs.emplace_back(Switchs[tmp]);
        }

        iter2 = iter->FirstChildElement("Assigned MSG");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Assigned MSG")) {
            iter2->QueryIntText(&tmp);
            EndSystems[i].Msg.emplace_back(msgs[tmp]);
            msgs[tmp].Sender = std::ref(EndSystems[i]);
        }

        iter2 = iter->FirstChildElement("Receiver for MSG");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Receiver for MSG")) {
            iter2->QueryIntText(&tmp);
            msgs[tmp].Receivers.insert(i);
        }
    }

    iter = config->FirstChildElement("Switch");
    for (size_t i = 0; iter != nullptr; i++, iter = iter->NextSiblingElement("Switch")) {
        Switchs[i].Num = i;
        int tmp;

        XMLElement* iter2 = iter->FirstChildElement("Connected with Link");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Connected with Link")) {
            PortGCL.emplace_back();
            iter2->QueryIntText(&tmp);
            Switchs[i].ConnectedLinks.emplace_back(Links[tmp]);
        }

        iter2 = iter->FirstChildElement("Connected with Switch");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Connected with Switch")) {
            iter2->QueryIntText(&tmp);
            Switchs[i].ConnectedSwitchs.emplace_back(Switchs[tmp]);
        }

        XMLElement* iter2 = iter->FirstChildElement("Connected with ES");
        for (; iter2 != nullptr; iter2 = iter2->NextSiblingElement("Connected with ES")) {
            iter2->QueryIntText(&tmp);
            Switchs[i].ConnectedEndSystems.emplace_back(EndSystems[tmp]);
        }
    }
}

double Network::AvgLength() {
    double tmp = 0;
    for (auto i& : Links) {
        if (UnusedLinks.contains(i)) {
            tmp += i.Length;
        }
    }
    return tmp / MaxLength;
}
