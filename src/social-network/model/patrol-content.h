#ifndef PATROL_CONTENT_H
#define PATROL_CONTENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"

using namespace std;

namespace ns3 {

class PatrolContent
{
    public:
        PatrolContent();
        virtual ~PatrolContent();

        uint32_t cellNumber;
        Ipv4Address producter;

        bool operator==(PatrolContent &obj);
};

}

#endif