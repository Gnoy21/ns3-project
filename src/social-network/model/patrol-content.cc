#include "patrol-content.h"

using namespace std;

namespace ns3 {

PatrolContent::PatrolContent()
{
    cellNumber = 0;
    producter = Ipv4Address("0.0.0.0");
}

PatrolContent::~PatrolContent()
{
    
}

bool PatrolContent::operator==(PatrolContent &obj) 
{ 
    if (this->cellNumber == obj.cellNumber)
    {
        return true;
    }

    return false;
    
}
}