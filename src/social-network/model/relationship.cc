#include "ns3/log.h"
#include "relationship.h"
#include "ns3/kmlsample.h"

#include <math.h>

namespace ns3
{

Relationship::Relationship (Ipv4Address peerID1):
    socialTableSize (0)
{
    nodeID = peerID1;
}

uint32_t
Relationship::GetSocialTableSize () const
{
    return socialTableSize;
}

int
Relationship::SearchTableEntry (Ipv4Address peer1, Ipv4Address peer2)
{
    for (uint16_t i=0; i < socialTableSize; i++)
    {
        if ((peer1 == socialTable[i].peerID1) && (peer2 == socialTable[i].peerID2))
            return i;
    }
    return -1;
}

double
Relationship::ComputeSocialTie (int index, Time baseTime)
{
    double F = 0;
    double lambda = 0.0001;
    double diffTime;
    
    // Compute time difference from encounter n and n-1
    diffTime = baseTime.GetSeconds () - socialTable[index].timestamp.GetSeconds (); 
    // Compute current weighing Function value
    F = pow(0.5, (diffTime * lambda));
    // Add weighing function to previous function
    return (F + socialTable[index].socialTieValue);
}


void
Relationship::UpdateSocialTable (Ipv4Address peerID2, Time time)
{
    SocialTableEntry s;
    s.peerID1 = nodeID; //this node ID
    s.peerID2 = peerID2; //encounter node ID
    s.timestamp = time;
    int index;

    // Determine if node was met in the past if less than 0 never met
    if ((index = SearchTableEntry (nodeID, peerID2)) < 0)
    {
        s.socialTieValue = 1;
        // Insert the encounter in the vector
        socialTable[socialTableSize] = s;
        socialTableSize = (socialTableSize + 1) % ARRAY_SIZE;
    }
    // node is in the vector
    else
    {
        // Determine socail tie value and place in vector
        s.socialTieValue = ComputeSocialTie (index, time);
        socialTable[index] = s;
    }
    
    // Swap the id's of nodes
    s.peerID1 = peerID2;
    s.peerID2 = nodeID;

    // Determine if node was met in the past if less than 0 never met
    if ((index = SearchTableEntry (peerID2, nodeID)) < 0)
    {
        s.socialTieValue = 1;
        // Insert the encounter in the vector
        socialTable[socialTableSize] = s;
        socialTableSize = (socialTableSize + 1) % ARRAY_SIZE;
    }
    // node is in the vector
    else
    {
        // Determine socail tie value and place in vector
        s.socialTieValue = ComputeSocialTie (index, time);
        socialTable[index] = s;
    }


}

void
Relationship::UpdateAndMergeSocialTable (Ipv4Address peerID2, Time time, SocialTableEntry *socialTableAddress, uint32_t size) {
    
    // Update the table
    UpdateSocialTable (peerID2, time);

    // Merge the updated tables
    MergeSocialTables(socialTableAddress, size);

    // Compute the centrality
    ComputeCentrality ();

    // Compute the socail tie level for the centrality table
    // This fails when K is less than or equal to the number of point must ensure that there are more points than K
    if (K_CLUSTER < centralityTableSize)
    {
        GenerateCenters (K_CLUSTER, centralityTable, centralityTableSize, centers);
    }
}


double
Relationship::GetSocialTie (Ipv4Address peer1, Ipv4Address peer2) const
{
    int index = -1;

    for (uint16_t i=0; i < socialTableSize; i++)
    {
        if ((peer1 == socialTable[i].peerID1) && (peer2 == socialTable[i].peerID2))
        {
            index = i;
            break;
        }
    }

    if (index < 0)
        return 0;
    else
        return socialTable[index].socialTieValue;
}

SocialTableEntry*
Relationship::GetSocialTableAddress()
{
    return socialTable;
}

CentralityTableEntry*
Relationship::GetCentralityTableAddress (uint32_t &size)
{
    size = centralityTableSize;

    return centralityTable;
}

int*
Relationship::GetCentralityArrayAddress (int &size)
{
    for (uint32_t i=0; i < centralityTableSize; i++)
    {
        centralityArray[i] = centralityTable[i].centrality * 1000;
    }

    size = centralityTableSize;

    return centralityArray;
}

uint32_t
Relationship::GetCentralityTableSize () const
{
    return centralityTableSize;
}

void
Relationship::MergeSocialTables (SocialTableEntry *peerTable, uint32_t size)
{
    uint16_t j;
    uint16_t initSize = socialTableSize;

    for (uint32_t i=0; i < size; i++)
    {
        for(j=0; j < initSize; j++)
        {
            // Search for mathcing entry in both peerTables
            if ((socialTable[j].peerID1 == (peerTable+i)->peerID1) && (socialTable[j].peerID2 == (peerTable+i)->peerID2))
            {
                // Compare timestamps
                if ((peerTable+i)->timestamp > socialTable[j].timestamp)
                {
                    // Replace if peer peerTable is more recent
                    socialTable[j] = *(peerTable+i);
                }
                break;
            }

            //TODO try and take this out of the loop
        }
        // If no previous match found add to social peerTable
        if (j == initSize)
        {
            socialTable[socialTableSize] = *(peerTable+i);
            socialTableSize = (socialTableSize + 1) % ARRAY_SIZE;
        }

    }
}

uint32_t
Relationship::UniqueNodeCount ()
{
    uint32_t localTableSize = 0; // Start table over every time centrality is computed
    uint32_t index;

    Ipv4Address *localTable = new Ipv4Address[socialTableSize];

    // Iterate through the social table and construct the centrality table
    for (uint32_t i=0; i < socialTableSize; i++)
    {
        // Look for matching node IDs in both tables
	    for (index = 0; index < localTableSize; index++)
        {
            if (socialTable[i].peerID1 == localTable[index])
                break;
        }

       if (index == localTableSize)
       {
            localTable[index] = socialTable[i].peerID1;
            localTableSize++;
       }
    }

    delete [] localTable;
    return localTableSize;

}

void
Relationship::ComputeCentrality ()
{    
    centralityTableSize = 0; // Start table over every time centrality is computed
    double alpha = 0.5; // 
    uint32_t index;
    CentralityTableEntry c;

    uint32_t nodeCount = UniqueNodeCount ();

    // Iterate through the social table and construct the centrality table
    for (uint32_t i=0; i < socialTableSize; i++)
    {
        // Look for matching node IDs in both tables
        for (index = 0; index < centralityTableSize; index++)
        {
            if (socialTable[i].peerID1 == centralityTable[index].nodeID)
                break;
        }

        // Insert centrality entry at index and increase centrality table size
        if (index == centralityTableSize)
        {
            double RkSum = 0; // social tie sum
            double RkSquSum = 0; // social tie sqared sum
            //uint16_t n = 0; // Number of nodes observed
            
            // Sum up the Rk values and increment n
            for (uint32_t j=i; j < socialTableSize; j++)
            {
                // Compare the id of candidate to every other node in table
                if (socialTable[i].peerID1 == socialTable[j].peerID1)
                {
                    RkSum = RkSum + socialTable[j].socialTieValue;
                    RkSquSum = RkSquSum + pow(socialTable[j].socialTieValue,2);
                    //n++;
                }
            }
             
            double equPt1;
            double equPt2;
            // First part of the equation
            //equPt1 = alpha * (RkSum)/n;
            equPt1 = alpha * (RkSum)/nodeCount;

            // Second part of the equation
            //equPt2 = (1 - alpha) * pow(RkSum,2) / (n * RkSquSum);
            equPt2 = (1 - alpha) * pow(RkSum,2) / (nodeCount * RkSquSum);
            
            // Insert Node ID into centrality entry
            c.nodeID = socialTable[i].peerID1;
            // Compute centrality for entry
            c.centrality = equPt1 + equPt2;
            
            centralityTable[index] = c; // Insert centrality entry
            centralityTableSize++; // Increment the size of of the centrality table because entry was made
        }
    }
}

Ipv4Address 
Relationship::GetHigherCentralityNode (Ipv4Address peer1, Ipv4Address peer2)
{
    double peer1Centrality = 0;
    double peer2Centrality = 0;

    Ipv4Address dummyAddress = "0.0.0.0"; // Returned if no node has higher value


    for (uint32_t i = 0; i < centralityTableSize; i++)
    {
        // Find the Social level of the peers
        if (peer1 == centralityTable[i].nodeID)
        {
            peer1Centrality = centralityTable[i].centrality;
        }
        if (peer2 == centralityTable[i].nodeID)
        {
            peer2Centrality = centralityTable[i].centrality;
        }
        // End loop if a level is found for both peers
        if ((peer1Centrality > 0) && (peer2Centrality > 0))
        {
            break;
        }
    }

    // If the peers are equal return the dummy address
    if (peer1Centrality == peer2Centrality)
    {
        return dummyAddress;
    }
    else if (peer1Centrality > peer2Centrality)
    {
        return peer1;
    }
    else
    {
        return peer2;
    }

}

void
Relationship::PrintSocialTable ()
{
    //if (nodeID == Ipv4Address("0.0.0.0"))
    if (true)
    {
        if (socialTableSize == 0)
        {
            std::cout << "There are no Social Ties for this node\n";
        }
        else
        {
            std::cout << "Peer1\t\t" << "Peer2\t\t" << "Time\t" << "SocialTie" << std::endl;

            for (uint32_t i=0; i < socialTableSize; i++)
            {
                std::cout << socialTable[i].peerID1 << "\t" << socialTable[i].peerID2 << "\t" << socialTable[i].timestamp.GetSeconds() 
                << "\t"  << socialTable[i].socialTieValue << std::endl;
            }
        }
    }
}

void
Relationship::PrintCentralityTable ()
{
    if (centralityTableSize == 0)
    {
        std::cout << "This node has no Centrality values\n";
    }
    else
    {
        for (uint32_t i=0; i < centralityTableSize; i++)
        {
            if(nodeID == centralityTable[i].nodeID)
            {
                std::cout << centralityTable[i].nodeID << " CentralityValue " << centralityTable[i].centrality << std::endl;
            }
        }
    }
}

void
Relationship::PrintSocialLevelTable ()
{
    if (centralityTableSize == 0)
    {
        std::cout << "This node has no Social Level Values values" << std::endl;
    }
    else
    {
        for (uint32_t i=0; i < centralityTableSize; i++)
        {
            if (nodeID == centralityTable[i].nodeID)
            {
                std::cout << centralityTable[i].nodeID << " SocialLevel " << centralityTable[i].SocialLevel << std::endl;
            }
        }
    }
}

Ipv4Address
Relationship::GetHigherSocialTie (Ipv4Address peer1, Ipv4Address peer2, Ipv4Address destination)
{
    double peer1SocialTie = 0;
    double peer2SocialTie = 0;

    Ipv4Address dummyAddress = "0.0.0.0"; // Returned if no node has higher value

    for (uint32_t index = 0; index < socialTableSize; index++)
    {
        // Check if the peers are in the social table
        if ((peer1 == socialTable[index].peerID1) && (destination == socialTable[index].peerID2))
        {
            peer1SocialTie = socialTable[index].socialTieValue;
        }
        
        else if ((peer2 == socialTable[index].peerID1) && (destination == socialTable[index].peerID2))
        {
            peer2SocialTie = socialTable[index].socialTieValue;
        }

        // End loop when a value is set for both peers
        if ((peer1SocialTie > 0) && (peer2SocialTie > 0))
        {
            break;
        }
    }

    // If the peers are equal return the dummy address
    if (peer1SocialTie == peer2SocialTie)
    {
        return dummyAddress;
    }
    else if (peer1SocialTie > peer2SocialTie)
    {
        return peer1;
    }
    else
    {
        return peer2;
    }

}

//peer 
Ipv4Address
Relationship::GetHigherSocialLevel (Ipv4Address peer1, Ipv4Address peer2)
{
    double peer1SocialLevel = 0;
    double peer2SocialLevel = 0;

    Ipv4Address dummyAddress = "0.0.0.0"; // Returned if no node has higher value


    for (uint32_t i = 0; i < centralityTableSize; i++)
    {
        // Find the Social level of the peers
        if (peer1 == centralityTable[i].nodeID)
        {
            peer1SocialLevel = centralityTable[i].SocialLevel;
        }
        if (peer2 == centralityTable[i].nodeID)
        {
            peer2SocialLevel = centralityTable[i].SocialLevel;
        }
        // End loop if a level is found for both peers
        if ((peer1SocialLevel > 0) && (peer2SocialLevel > 0))
        {
            break;
        }
    }

    // If the peers are equal return the dummy address
    if (peer1SocialLevel == peer2SocialLevel)
    {
        return dummyAddress;
    }
    else if (peer1SocialLevel > peer2SocialLevel)
    {
        return peer1;
    }
    else
    {
        return peer2;
    }
}

Ipv4Address*
Relationship::GetHigherCentralityNodes (uint32_t &size)
{
    double centrality = -1;
    size = 0;
    // Find the nodes centrality
    for (uint32_t i = 0; i < centralityTableSize; i++)
    {
        if (nodeID == centralityTable[i].nodeID)
        {
            centrality = centralityTable[i].centrality;
            break;
        }
    }

    if (centrality > 0)
    {
        for (uint32_t i=0; i < centralityTableSize; i++)
        {
            if (centrality < centralityTable[i].centrality)
            {
                higherCentralityTable[size] = centralityTable[i].nodeID;
                size++;
            }
        }
    }
    // Return the beginning of the table
    return  higherCentralityTable;
}

} // namespace ns3
