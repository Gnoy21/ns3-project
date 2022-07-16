#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include "ns3/nstime.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"


#define ARRAY_SIZE 20000
#define K_CLUSTER 2 // FIXME

namespace ns3
{

struct SocialTableEntry
{
    SocialTableEntry ():
    peerID1 ("0.0.0.0"), peerID2 ("0.0.0.0"),
        timestamp (Seconds(0)), socialTieValue (0)
    {
    }
    Ipv4Address peerID1;
    Ipv4Address peerID2;
    Time timestamp;
    double socialTieValue;
};

struct CentralityTableEntry
{
    CentralityTableEntry():
    nodeID ("0.0.0.0"), centrality (0), SocialLevel (0)
    {
    }
    Ipv4Address nodeID;
    double centrality;
    int SocialLevel;
};

class Relationship
{
public:
    //CentralityTableEntry cte;
    Relationship (Ipv4Address myID);
    void UpdateAndMergeSocialTable (Ipv4Address peerID, Time time,
            SocialTableEntry *socialTableAddress, uint32_t size);
    double GetSocialTie (Ipv4Address peer1, Ipv4Address peer2) const;
    uint32_t GetSocialTableSize () const;
    uint32_t GetCentralityTableSize () const;
    Ipv4Address *GetHigherCentralityNodes (uint32_t &size);
    //CentralityTableEntry *GetHigherCentralityNodes (uint32_t &size);
    CentralityTableEntry *GetCentralityTableAddress (uint32_t &size); // Use this for the entire table
    SocialTableEntry *GetSocialTableAddress ();
    int *GetCentralityArrayAddress (int &size); // Use this one for Array of integers
    Ipv4Address GetHigherSocialTie (Ipv4Address peer1, Ipv4Address peer2, Ipv4Address destination); // Return the peer with higher social tie to the destination
    Ipv4Address GetHigherSocialLevel (Ipv4Address peer1, Ipv4Address peer2); // Return peer with the higher social level
    Ipv4Address GetHigherCentralityNode (Ipv4Address peer1, Ipv4Address peer2);

    // Test functions
    void PrintCentralityTable ();
    void PrintSocialTable ();
    void PrintSocialLevelTable ();
   
private:
    void UpdateSocialTable (Ipv4Address peerID, Time time);
    void MergeSocialTables(SocialTableEntry* peerTable, uint32_t size);
    int SearchTableEntry (Ipv4Address peer1, Ipv4Address peer2);
    double ComputeSocialTie (int index, Time time);
    void ComputeCentrality ();
    uint32_t UniqueNodeCount ();
    void printSocialTable1();


private:
    SocialTableEntry socialTable [ARRAY_SIZE];
    CentralityTableEntry centralityTable [ARRAY_SIZE];
    //CentralityTableEntry higherCentralityTable [ARRAY_SIZE/2];
    Ipv4Address higherCentralityTable [ARRAY_SIZE/2];
    double centrality;
    uint32_t socialTableSize;
    uint32_t centralityTableSize;
    int centralityArray [ARRAY_SIZE];
    double centers [K_CLUSTER];

    Ipv4Address nodeID;
};

} // namespace ns3

#endif // RELATIONSHIP_H
