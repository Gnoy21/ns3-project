#include "social-network.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mobility-module.h"
#include "ns3/paparazzi-mobility-model.h"

#include "ns3/string.h"
#include "ns3/pkt-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4.h"

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SocialNetworkApplication");
NS_OBJECT_ENSURE_REGISTERED (SocialNetwork);

SocialNetwork::SocialNetwork ()
{
    NS_LOG_FUNCTION (this);
    m_sent = 0;
    m_socket = 0;
    m_sendEvent = EventId ();
    m_contentManager = new ContentManager();
    m_interestManager = new InterestManager();
    m_interestBroadcastId = 0;
    m_pending_data = new vector<PendingDataEntry>;
    m_pending_interest_known_content_provider = new vector<PendingInterestEntryKnownContentProvider>;
    m_pending_interest_unknown_content_provider = new vector<PendingInterestEntryUnknownContentProvider>;
    m_counter = 0;
    m_previousEncounterNode = Ipv4Address("0.0.0.0");
    m_previousEncounterTime = 0;
    m_firstSuccess = false;
    m_recivePktCounter = 0;
    m_receiveDataPkt = 0;
    m_delay = 0;
    m_sequenceNumber = 1;

    m_initialRequestedContent = 101;
}


SocialNetwork::~SocialNetwork()
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;

    cout << "================================" << endl;

    cout << m_ownIpAddress << " ReceivePktCounter " << m_recivePktCounter << endl;

    m_relationship->PrintCentralityTable();
    m_relationship->PrintSocialLevelTable();

    if (m_receiveDataPkt != 0)
    {
        cout << m_ownIpAddress << " ReceiveDataPkt " << m_receiveDataPkt << endl;
    }

    uint32_t temp_int = 0;

    for(auto i:m_receive_data)
    {
        if(i.firstReceive)
        {
            m_delay = m_delay + i.delay;
            temp_int++;
        }
    }

    if(temp_int != 0)
    {
        m_delay = m_delay / temp_int;
    }

    if(m_delay != 0)
    {
        cout << m_ownIpAddress << " Delay " << m_delay << endl;
    }
    
    if (m_sequenceNumber != 1)
    {
        cout << m_ownIpAddress << " SendInterestPkt " << m_sequenceNumber-1 << endl;
    }

    delete m_contentManager;
    delete m_interestManager;
    delete m_relationship;
    delete m_pending_data;
    delete m_pending_interest_known_content_provider;
    delete m_pending_interest_unknown_content_provider;
}

void
SocialNetwork::DoDispose (void)
{
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
}


void
SocialNetwork::Setup (uint16_t port)
{
    m_peerPort = port;
}

void
SocialNetwork::RequestContent (uint32_t content)
{
    m_initialRequestedContent = content;
    PendingInterestEntryUnknownContentProvider entry;
    entry.requester = GetNodeAddress();
    entry.broadcastId = m_interestBroadcastId;
    entry.requestedContent = m_initialRequestedContent;
    m_pending_interest_unknown_content_provider->push_back(entry);
        
    m_interestBroadcastId++;
}

void
SocialNetwork::SetContent (uint32_t content)
{
    m_ownIpAddress = GetNodeAddress();

    PatrolContent patrolContent;
    patrolContent.cellNumber = content;
    patrolContent.producter = m_ownIpAddress;

    m_contentManager->Insert(patrolContent);
}


void 
SocialNetwork::StopApplication ()
{

    NS_LOG_FUNCTION (this);

    if (m_socket != 0) 
    {
        m_socket->Close ();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        m_socket = 0;
    }

    Simulator::Cancel (m_sendEvent);
}

void SocialNetwork::StartApplication (void) {
    NS_LOG_FUNCTION (this);
    Ipv4Address broadcastAddr = Ipv4Address("255.255.255.255");
    mob = GetNode()->GetObject<PaparazziMobilityModel>();
    
    m_ownIpAddress = GetNodeAddress();
    m_relationship = new Relationship(m_ownIpAddress);
  
    if (m_socket == 0) {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);
        m_socket->SetAllowBroadcast(true);
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_peerPort);
        m_socket->Bind (local);
        m_socket->Connect (InetSocketAddress (broadcastAddr, m_peerPort));
    }

    m_socket->SetRecvCallback (MakeCallback (&SocialNetwork::HandleRead, this));
    
    ScheduleEvents();
}

void
SocialNetwork::ScheduleEvents()
{
    SendHello();
    CheckCell();
}

void 
SocialNetwork::SendHello ()
{
    PktHeader *header = CreateHelloPacketHeader();
    SendPacket(*header);
}

void
SocialNetwork::SendPacket(PktHeader header)
{
    Ptr<Packet> p = Create<Packet> ();
    p->AddHeader(header);

    if (m_socket != 0)
    {
        m_socket->Send (p);
    }
}

void
SocialNetwork::CheckCell()
{
    if(mob==0)
    {

        Simulator::Schedule(Seconds(1), &SocialNetwork::ScheduleEvents, this);
        
        return;
    }
    uint32_t *m_finishedPatrol = mob->GetFinishedPatrol();
    int counter = mob->GetFinishedPatrolSize();

    if(counter > m_counter)
    {
        for(int i = m_counter; i < counter; i++)
        {
            PatrolContent patrolContent;
            patrolContent.cellNumber = m_finishedPatrol[i];
            patrolContent.producter = m_ownIpAddress;

            m_contentManager->Insert(patrolContent);
        }

        m_counter = counter;
        
    }
    Simulator::Schedule(Seconds(1), &SocialNetwork::ScheduleEvents, this);
}

PktHeader*
SocialNetwork::CreateDataPacketHeader(Ipv4Address requester, Ipv4Address destination, uint32_t broadcastId, uint32_t requestedContent, double timeStamp, uint32_t sequenceNumber)
{
    /*
    cout << "CreateDataPacketHeader's requester : " << requester << endl;
    cout << "CreateDataPacketHeader's Currunt Node : " << GetNodeAddress() << endl;
    cout << "CreateDataPacketHeader's timeStamp : " << timeStamp << endl;
    cout << "CreateDataPacketHeader's sequenceNumber : " << sequenceNumber << endl;
    */

    PktHeader *header = new PktHeader();
    header->SetSource(GetNodeAddress());
    header->SetDestination(destination);
    header->SetRequestedContent(requestedContent);

    header->SetRequesterId(requester);
    header->SetInterestBroadcastId(broadcastId);
    header->SetPacketType(DATA);
    header->SetTimeStamp(timeStamp);
    header->SetSequenceNumber(sequenceNumber);
    
    return header;
}

PktHeader* 
SocialNetwork::CreateInterestPacketHeaderUnknownContentProvider(Ipv4Address requester, Ipv4Address destination, uint32_t broadcastId, uint32_t requestedContent, double timeStamp, uint32_t sequenceNumber)
{
    /*
    cout << "CreateInterestPacketHeaderUnknownContentProvider's requester : " << requester << endl;
    cout << "CreateInterestPacketHeaderUnknownContentProvider's Currunt Node : " << GetNodeAddress() << endl;
    cout << "CreateInterestPacketHeaderUnknownContentProvider's timeStamp : " << timeStamp << endl;
    cout << "CreateInterestPacketHeaderUnknownContentProvider's sequenceNumber : " << sequenceNumber << endl;
    */

    PktHeader *header = new PktHeader();
    header->SetSource(GetNodeAddress());
    header->SetDestination(destination);
    header->SetRequestedContent(requestedContent);
    header->SetRequesterId(requester); 
    header->SetInterestBroadcastId(broadcastId);
    header->SetPacketType(InterestUnknownContentProvider);
    header->SetTimeStamp(timeStamp);
    header->SetSequenceNumber(sequenceNumber);
    
    if (requester == GetNodeAddress())
    {
        ReceivedDataPacket data;
        data.sequenceNumber = m_sequenceNumber;
        data.firstReceive = false;
        data.delay = 0;
        m_receive_data.push_back(data);
        m_sequenceNumber++;
    }
    
    return header;
}

PktHeader*
SocialNetwork::CreateInterestPacketHeaderKnownContentProvider(Ipv4Address requester, Ipv4Address destination, uint32_t broadcastId, uint32_t requestedContent, Ipv4Address contentProvider, double timeStamp, uint32_t sequenceNumber) 
{
    /*
    cout << "CreateInterestPacketHeaderKnownContentProvider's requester : " << requester << endl;
    cout << "CreateInterestPacketHeaderKnownContentProvider's Currunt Node : " << GetNodeAddress() << endl;
    cout << "CreateInterestPacketHeaderKnownContentProvider's timeStamp : " << timeStamp << endl;
    cout << "CreateInterestPacketHeaderKnownContentProvider's sequenceNumber : " << sequenceNumber << endl;
    */

    PktHeader *header = new PktHeader();
    header->SetSource(GetNodeAddress());
    header->SetDestination(destination);
    header->SetRequestedContent(requestedContent);
    header->SetRequesterId(requester); 
    header->SetInterestBroadcastId(broadcastId);
    header->SetPacketType(InterestKnownContentProvider);
    header->SetContentProviderId(contentProvider);
    header->SetTimeStamp(timeStamp);
    header->SetSequenceNumber(sequenceNumber);

    if (requester == GetNodeAddress())
    {
        ReceivedDataPacket data;
        data.sequenceNumber = m_sequenceNumber;
        data.firstReceive = false;
        data.delay = 0;
        m_receive_data.push_back(data);
        m_sequenceNumber++;
    }
    
    return header;
}


PktHeader* 
SocialNetwork::CreateHelloPacketHeader() 
{
    PktHeader *header = new PktHeader();
    header->SetSocialTieTable( (uint64_t *)(m_relationship->GetSocialTableAddress()) );
    header->SetSocialTieTableSize(m_relationship->GetSocialTableSize());
    header->SetPacketType(HELLO);
    header->SetDestination(Ipv4Address("255.255.255.255"));
    header->SetSource(GetNodeAddress());

    return header;
}


PktHeader *
SocialNetwork::CreateDigestPacketHeader(Ipv4Address destinationId)
{
    PktHeader *header = new PktHeader();
    header->SetContentArraySize(m_contentManager->GetContentArraySize());
    header->SetContentArray( m_contentManager->GetContentArray() );
    header->SetPacketType(DIGEST);
    header->SetDestination(destinationId);
    header->SetSource(GetNodeAddress());
    
    return header;
}


Ipv4Address
SocialNetwork::GetNodeAddress()
{
    Ptr<Node> node = GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    Ipv4Address thisIpv4Address = ipv4->GetAddress(1,0).GetLocal();
    return thisIpv4Address;
}


void
SocialNetwork::PrintAllContent(PatrolContent *array, uint32_t size)
{
    for (uint32_t i=0;i<size;i++)
    {
        std::cout << array[i].cellNumber;
        if(i != size-1)
        {
            std::cout << ",";
        }
    }
    std::cout << endl;
}

void SocialNetwork::HandleRead (Ptr<Socket> socket) 
{
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom (from)))
    {
        PktHeader *header = new PktHeader();
        packet->PeekHeader(*header);
        PacketType packetType = header->GetPacketType();

        /*
        if(m_previousEncounterNode == header->GetSource() && Simulator::Now().GetSeconds()-m_previousEncounterTime < 100 && packetType==HELLO)
        {
            return;
        }
        */

        m_recivePktCounter++;
        m_previousEncounterNode = header->GetSource();
        m_previousEncounterTime = Simulator::Now().GetSeconds();

        switch (packetType)
        {
            case HELLO:
                HandleHello(header);
                break;
                
            case DATA:
                HandleData(header);
                break;
                
            case DIGEST:
                HandleDigest(header);
                break;
                
            case InterestUnknownContentProvider:
                HandleInterestUnknownContentProvider(header);
                break;
                
            case InterestKnownContentProvider:
                HandleInterestKnownContentProvider(header);
                break;
                
            default:
                return;
        }
   }
}

void
SocialNetwork::HandleDigest(PktHeader *header)
{
    Ipv4Address currentNode = GetNodeAddress();

    if ( (currentNode == (header->GetDestination())) )
    {
        uint32_t contentArraySize = header->GetContentArraySize();
        PatrolContent *contentArray = header->GetContentArray();

        m_contentManager->Merge(contentArray, contentArraySize);
    }
}


void
SocialNetwork::HandleHello(PktHeader *header)
{
    Ipv4Address encounterNode = header->GetSource();
    SocialTableEntry *socialTieTableEntry = (SocialTableEntry *) (header->GetSocialTieTable());
    uint32_t socialTieTableSize = header->GetSocialTieTableSize();

    m_relationship->UpdateAndMergeSocialTable(encounterNode,
                                           Simulator::Now(),
                                           socialTieTableEntry,
                                           socialTieTableSize);

    DecideWhetherToSendContentNameDigest(header);

    ProcessPendingData(header);
    ProcessPendingInterestKnownContentProvider(header);
    ProcessPendingInterestUnknownContentProvider(header); 
}

void
SocialNetwork::DecideWhetherToSendContentNameDigest(PktHeader *header)
{
    Ipv4Address currentNode = GetNodeAddress();
    Ipv4Address encounterNode = header->GetSource();
    Ipv4Address higherCentralityNode = m_relationship->GetHigherCentralityNode(currentNode, encounterNode);
    
    if (higherCentralityNode == encounterNode)
    {
        PktHeader *header = CreateDigestPacketHeader(encounterNode);
        SendPacket(*header);
    }
}


void
SocialNetwork::ProcessPendingInterestKnownContentProvider(PktHeader *header)
{
    //cout << "Inside ProcessPendingInterestKnownContentProvider" << endl;

    Ipv4Address currentNode = GetNodeAddress();
    Ipv4Address encounterNode = header->GetSource();

    //cout << "ProcessPendingInterestKnownContentProvider's pending size : " << m_pending_interest_known_content_provider->size() << endl;
    
    for (vector<PendingInterestEntryKnownContentProvider>::iterator it = m_pending_interest_known_content_provider->begin(); it != m_pending_interest_known_content_provider->end(); ++it)
    {
    
        //encounter node is the content provider
        if ( (it->contentProvider) == encounterNode )
        {
            PktHeader *header = CreateInterestPacketHeaderKnownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->contentProvider, it->timestamp, it->sequenceNumber);
            SendPacket(*header);
        }
        else
        {
            Ipv4Address higherSocialTieNode = m_relationship->GetHigherSocialTie(currentNode, encounterNode, it->contentProvider);
            if (higherSocialTieNode == encounterNode)
            {
                if ( (it->lastRelayNode) == Ipv4Address("0.0.0.0") )
                {
                    it->lastRelayNode = encounterNode;
                    
                    if (it->requester == GetNodeAddress())
                    {
                        PktHeader *header = CreateInterestPacketHeaderKnownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->contentProvider, Simulator::Now ().GetSeconds (), m_sequenceNumber);
                        SendPacket(*header);
                    }
                    else
                    {
                        PktHeader *header = CreateInterestPacketHeaderKnownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->contentProvider, it->timestamp, it->sequenceNumber);
                        SendPacket(*header);
                    }
                }
                else
                {
                    Ipv4Address higherSocialTieNode = m_relationship->GetHigherSocialTie(encounterNode, it->lastRelayNode, it->contentProvider);
                    if ( higherSocialTieNode == encounterNode )
                    {
                        it->lastRelayNode = encounterNode;
                        
                        if (it->requester == GetNodeAddress())
                        {
                            PktHeader *header = CreateInterestPacketHeaderKnownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->contentProvider, Simulator::Now ().GetSeconds (), m_sequenceNumber);
                            SendPacket(*header);
                        }
                        else
                        {
                            PktHeader *header = CreateInterestPacketHeaderKnownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->contentProvider, it->timestamp, it->sequenceNumber);
                            SendPacket(*header);
                        }
                    }
                }
            }
        }
    }
}


void
SocialNetwork::ProcessPendingInterestUnknownContentProvider(PktHeader *header)
{
    //cout << "Inside ProcessPendingInterestUnknownContentProvider" << endl;

    Ipv4Address currentNode = GetNodeAddress();
    Ipv4Address encounterNode = header->GetSource();
    
    //cout << "ProcessPendingInterestUnknownContentProvider's pending size : " << m_pending_interest_unknown_content_provider->size() << endl;

    for (vector<PendingInterestEntryUnknownContentProvider>::iterator it = m_pending_interest_unknown_content_provider->begin(); it != m_pending_interest_unknown_content_provider->end(); ++it)
    {
        Ipv4Address higherSocialLevelNode = m_relationship->GetHigherSocialLevel(currentNode, encounterNode);
        if ( higherSocialLevelNode == encounterNode )
        {
            if ( (it->lastRelayNode) == Ipv4Address("0.0.0.0") )
            {
                it->lastRelayNode = encounterNode;

                if (it->requester == GetNodeAddress())
                {
                    PktHeader *header = CreateInterestPacketHeaderUnknownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, Simulator::Now ().GetSeconds (), m_sequenceNumber);
                    SendPacket(*header);
                }
                else
                {
                    PktHeader *header = CreateInterestPacketHeaderUnknownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->timestamp, it->sequenceNumber);
                    SendPacket(*header);
                }
            }
            else
            {
                Ipv4Address higherSocialLevelNode = m_relationship->GetHigherSocialLevel(encounterNode, it->lastRelayNode);
                if ( higherSocialLevelNode == encounterNode )
                {
                    it->lastRelayNode = encounterNode;
                
                    if (it->requester == GetNodeAddress())
                    {
                        PktHeader *header = CreateInterestPacketHeaderUnknownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, Simulator::Now ().GetSeconds (), m_sequenceNumber);
                        SendPacket(*header);
                    }
                    else
                    {
                        PktHeader *header = CreateInterestPacketHeaderUnknownContentProvider(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->timestamp, it->sequenceNumber);
                        SendPacket(*header);
                    }
                }
            }
        }
    }
}


void
SocialNetwork::ProcessPendingData(PktHeader *header)
{   
    //cout << "Inside ProcessPendingData" << endl;

    Ipv4Address currentNode = GetNodeAddress();
    Ipv4Address encounterNode = header->GetSource();

    //cout << "ProcessPendingData's pending size : " << m_pending_data->size() << endl;

    for (vector<PendingDataEntry>::iterator it = m_pending_data->begin(); it != m_pending_data->end(); ++it)
    {
        //encounter is the requester
        if ( (it->requester) == encounterNode )
        {
            PktHeader *header = CreateDataPacketHeader(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->timestamp, it->sequenceNumber);
            SendPacket(*header);
        }
        else 
        {
            Ipv4Address higherSocialTieNode = m_relationship->GetHigherSocialTie(currentNode, encounterNode, it->requester);
            if (higherSocialTieNode == encounterNode)
            {
                if ( (it->lastRelayNode) == Ipv4Address("0.0.0.0") )
                {
                    it->lastRelayNode = encounterNode;
                    
                    
                    PktHeader *header = CreateDataPacketHeader(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->timestamp, it->sequenceNumber);
                    SendPacket(*header);
                }
                else
                {
                    Ipv4Address higherSocialTieNode = m_relationship->GetHigherSocialTie(encounterNode, it->lastRelayNode, it->requester);
                    if( higherSocialTieNode == encounterNode )
                    {
                        it->lastRelayNode = encounterNode;
                        
                        PktHeader *header = CreateDataPacketHeader(it->requester, encounterNode, it->broadcastId, it->requestedContent, it->timestamp, it->sequenceNumber);
                        SendPacket(*header);
                    }
                }
            }
        }
    }
}

void
SocialNetwork::HandleInterestKnownContentProvider(PktHeader *header) 
{
    //cout << "Inside HandleInterestKnownContentProvider" << endl;

    Ipv4Address currentNode = GetNodeAddress();
    Ipv4Address encounterNode = header->GetSource();

    /*
    cout << "requester : " << header->GetRequesterId() << endl;
    cout << "encounter node : " << encounterNode << endl;
    cout << "current node : " << currentNode << endl;
    cout << "sequence number : " << header->GetSequenceNumber() << endl;
    cout << "timestamp : " << header->GetTimeStamp() << endl;
    */
    
    if ( !(currentNode == (header->GetDestination())) )
    {
        //I am not the node this Interest packet is destined to
        return;
    }

    Ipv4Address requester = header->GetRequesterId();    
    uint32_t requestedContent = header->GetRequestedContent();
    Ipv4Address contentProvider = header->GetContentProviderId();
    uint32_t broadcastId = header->GetInterestBroadcastId();
    
    InterestEntry interestEntry(requester, broadcastId, requestedContent, header->GetSequenceNumber());
    
    if (! (m_interestManager->Exist(interestEntry)) )
    {
        m_interestManager->Insert(interestEntry);
    
        if (contentProvider == m_ownIpAddress) //I am the content provider
        {
            if (encounterNode == requester)
            {
                PktHeader *header1 = CreateDataPacketHeader(requester, encounterNode, broadcastId, requestedContent, header->GetTimeStamp(), header->GetSequenceNumber());
                SendPacket(*header1);
            }
            else
            {
                PendingDataEntry entry;
                entry.requester = requester;
                entry.broadcastId = broadcastId;
                entry.requestedContent = requestedContent;
                entry.provider = m_ownIpAddress;
                entry.sequenceNumber = header->GetSequenceNumber();
                entry.timestamp = header->GetTimeStamp();
                m_pending_data->push_back(entry);
            }
        }
        else // I do not have the content, so I am a relay node to a known content provider
        {
            PendingInterestEntryKnownContentProvider entry;
            entry.requester = requester;
            entry.broadcastId = broadcastId;
            entry.requestedContent = requestedContent;
            entry.contentProvider = contentProvider;
            entry.timestamp = header->GetTimeStamp();
            entry.sequenceNumber = header->GetSequenceNumber();
            m_pending_interest_known_content_provider->push_back(entry);
        }
    }
    else
    {

    }
}

void SocialNetwork::HandleInterestUnknownContentProvider(PktHeader *header) 
{    
    //cout << "Inside HandleInterestUnknownContentProvider" << endl;

    Ipv4Address currentNode = GetNodeAddress();
    Ipv4Address encounterNode = header->GetSource();

    /*
    cout << "requester : " << header->GetRequesterId() << endl;
    cout << "encounter node : " << encounterNode << endl;
    cout << "current node : " << currentNode << endl;
    cout << "sequence number : " << header->GetSequenceNumber() << endl;
    cout << "timestamp : " << header->GetTimeStamp() << endl;
    */
    
    if ( !(currentNode == (header->GetDestination())) )
    {
        //I am not the node this Interest packet is destined to
        return;
    }

    Ipv4Address requester = header->GetRequesterId();    
    uint32_t requestedContent = header->GetRequestedContent();
    uint32_t broadcastId = header->GetInterestBroadcastId();

    InterestEntry interestEntry(requester, broadcastId, requestedContent, header->GetSequenceNumber());
    
    if (! (m_interestManager->Exist(interestEntry)) )
    {
        // First time sees this Interest packet
        m_interestManager->Insert(interestEntry);

        if (m_contentManager->Exist(requestedContent) && m_contentManager->SearchProvider(requestedContent) == m_ownIpAddress) //I am the content provider
        {
            if (encounterNode == requester)
            {
                PktHeader *header1 = CreateDataPacketHeader(requester, encounterNode, broadcastId, requestedContent, header->GetTimeStamp(), header->GetSequenceNumber());
                SendPacket(*header1);
            }
            else
            {
                PendingDataEntry entry;
                entry.requester = requester;
                entry.broadcastId = broadcastId;
                entry.requestedContent = requestedContent;
                entry.provider = m_ownIpAddress;
                entry.sequenceNumber = header->GetSequenceNumber();
                entry.timestamp = header->GetTimeStamp();
                m_pending_data->push_back(entry);
            }
        }
        else // I do not have the content
        {
            if (m_contentManager->Exist(requestedContent)) //I know which node has the content
            {
                PendingInterestEntryKnownContentProvider entry;
                entry.requester = requester;
                entry.broadcastId = broadcastId;
                entry.requestedContent = requestedContent;
                entry.contentProvider = m_contentManager->SearchProvider(requestedContent);
                entry.sequenceNumber = header->GetSequenceNumber();
                entry.timestamp = header->GetTimeStamp();
                m_pending_interest_known_content_provider->push_back(entry);
            }
            else // I do not know who has the content
            {
                PendingInterestEntryUnknownContentProvider entry;
                entry.requester = requester;
                entry.broadcastId = broadcastId;
                entry.requestedContent = requestedContent;
                entry.sequenceNumber = header->GetSequenceNumber();
                entry.timestamp = header->GetTimeStamp();
                m_pending_interest_unknown_content_provider->push_back(entry);
            }
        }
    }
    else
    {

    }
}

void
SocialNetwork::HandleData(PktHeader *header)
{
    //cout << "Inside HandleData" << endl;

    Ipv4Address currentNode = GetNodeAddress();
    
    if ( !(currentNode == header->GetDestination()) )
    {
        //I am not the node this Data packet is destined to
        return;
    }
    
    Ipv4Address requester = header->GetRequesterId();
    uint32_t requestedContent = header->GetRequestedContent();
    uint32_t broadcastId = header->GetInterestBroadcastId();

    /*
    cout << "requester : " << header->GetRequesterId() << endl;
    cout << "current node : " << currentNode << endl;
    cout << "sequence number : " << header->GetSequenceNumber() << endl;
    cout << "timestamp : " << header->GetTimeStamp() << endl;
    */
    
    if ( m_initialRequestedContent == requestedContent )
    {
        for(ReceivedDataPacket& i:m_receive_data)
        {
            if(!i.firstReceive && i.sequenceNumber == header->GetSequenceNumber())
            {
                i.delay = Simulator::Now ().GetSeconds () - header->GetTimeStamp();
                //cout << "delay : " << i.delay << endl;

                PatrolContent patrolContent;
                patrolContent.cellNumber = requestedContent;
                patrolContent.producter = m_ownIpAddress;

                m_contentManager->Insert(patrolContent);


                i.firstReceive = true;
                m_receiveDataPkt++;
            }
        }   
    }
    else
    {
        for (vector<PendingDataEntry>::iterator it = m_pending_data->begin(); it != m_pending_data->end(); ++it)
        {
            if ( (requester == it->requester) && (broadcastId == it->broadcastId) )
            {
                return;
            }
        }
            
        PendingDataEntry entry;
        entry.requester = requester;
        entry.broadcastId = broadcastId;
        entry.requestedContent = requestedContent;
        entry.provider = Ipv4Address("0.0.0.0");
        entry.sequenceNumber = header->GetSequenceNumber();
        entry.timestamp = header->GetTimeStamp();
        m_pending_data->push_back(entry);
    }
}
    
}

