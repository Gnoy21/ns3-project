/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SOCIAL_NETWORK_H
#define SOCIAL_NETWORK_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"

#include "pkt-header.h"
#include "content-manager.h"
#include "ns3/relationship.h"
#include "interest-manager.h"
#include "patrol-content.h"
#include "ns3/paparazzi-mobility-model.h"

using namespace std;

namespace ns3 {

struct PendingInterestEntryUnknownContentProvider
{
    PendingInterestEntryUnknownContentProvider()
    {
        lastRelayNode = Ipv4Address("0.0.0.0");
    }

    Ipv4Address lastRelayNode;
    Ipv4Address requester;
    uint32_t broadcastId;
    uint32_t requestedContent;
    uint32_t sequenceNumber;
    double timestamp;
};

struct PendingDataEntry
{
    PendingDataEntry()
    {
        lastRelayNode = Ipv4Address("0.0.0.0");
    }
    
    Ipv4Address lastRelayNode;
    Ipv4Address requester;
    uint32_t broadcastId;
    uint32_t requestedContent;
    Ipv4Address provider;
    uint32_t sequenceNumber;
    double timestamp;
};

struct PendingInterestEntryKnownContentProvider
{
    PendingInterestEntryKnownContentProvider()
    {
        lastRelayNode = Ipv4Address("0.0.0.0");
    }
    
    Ipv4Address lastRelayNode;
    Ipv4Address requester;
    uint32_t broadcastId;
    uint32_t requestedContent;
    Ipv4Address contentProvider;
    uint32_t sequenceNumber;
    double timestamp;
};

struct ReceivedDataPacket
{
    uint32_t sequenceNumber;
    double delay;
    bool firstReceive;
};

class Socket;
class Packet;

/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class SocialNetwork : public Application 
{
public:
    SocialNetwork ();
    virtual ~SocialNetwork ();

    // Avoid using the helper to setup an app
    void Setup (uint16_t port);
    // Set content requestor
    void RequestContent (uint32_t content);
    void SetContent (uint32_t content);

protected:
    virtual void DoDispose (void);

private:

    virtual void StartApplication (void);
    virtual void StopApplication (void);
    void SendHello ();
    void CheckCell();
    void CheckPatrol();
    void SendPacket(PktHeader header);
    void ScheduleEvents();
    Ipv4Address GetNodeAddress(void);
    
    void HandleRead (Ptr<Socket> socket);
    void HandleData(PktHeader *header);
    void HandleHello(PktHeader *header);
    void HandleDigest(PktHeader *header);
    void HandleInterestUnknownContentProvider(PktHeader *header);
    void HandleInterestKnownContentProvider(PktHeader *header);
    
    PktHeader *CreateDataPacketHeader(Ipv4Address requester, Ipv4Address destination, uint32_t broadcastId, uint32_t requestedContent, double timeStamp, uint32_t sequenceNumber);
    PktHeader *CreateHelloPacketHeader();
    PktHeader *CreateInterestPacketHeaderUnknownContentProvider(Ipv4Address requester, Ipv4Address destination, uint32_t broadcastId, uint32_t requestedContent, double timeStamp, uint32_t sequenceNumber);
    PktHeader *CreateInterestPacketHeaderKnownContentProvider(Ipv4Address requester, Ipv4Address destination, uint32_t broadcastId, uint32_t requestedContent, Ipv4Address contentProvider, double timeStamp, uint32_t sequenceNumber);
    PktHeader *CreateDigestPacketHeader(Ipv4Address destinationId);
                
    void ProcessPendingData(PktHeader *header);
    void ProcessPendingInterestKnownContentProvider(PktHeader *header);
    void ProcessPendingInterestUnknownContentProvider(PktHeader *header);
    void setFinishPatrol(uint8_t cell_number);
    
    //Print content line by line
    void PrintAllContent(PatrolContent *array, uint32_t size);
    void DecideWhetherToSendContentNameDigest(PktHeader *header);
    
    uint32_t m_count;
    Time m_interval;
    uint32_t m_size;
    int m_counter;
    int m_recivePktCounter;

    uint32_t m_sent;
    Ptr<Socket> m_socket;
    Address m_peerAddress;
    uint16_t m_peerPort;
    EventId m_sendEvent;
    /// Callbacks for tracing thce packet Tx events
    TracedCallback<Ptr<const Packet> > m_txTrace;
    
    Relationship *m_relationship;
    ContentManager *m_contentManager;
    InterestManager *m_interestManager;
    uint32_t m_interestBroadcastId;

    uint32_t m_receiveDataPkt;

    Ipv4Address m_previousEncounterNode;
    double m_previousEncounterTime;
    
    vector<PendingInterestEntryKnownContentProvider> *m_pending_interest_known_content_provider;
    vector<PendingInterestEntryUnknownContentProvider> *m_pending_interest_unknown_content_provider;
    vector<PendingDataEntry> *m_pending_data;
    vector<ReceivedDataPacket> m_receive_data;

    Ipv4Address m_ownIpAddress;

    double m_delay;

    Ptr<PaparazziMobilityModel> mob;
    
    bool m_firstSuccess; //for accounting purpose

    uint32_t m_initialRequestedContent;
    double m_firstSendInterestTime;

    uint32_t m_sequenceNumber;
    
};

}

#endif /* SOCIAL_NETWORK_H */