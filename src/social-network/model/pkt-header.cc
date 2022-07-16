#include "ns3/log.h"
#include "pkt-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PktHeader");
NS_OBJECT_ENSURE_REGISTERED (PktHeader);

std::string
GetPacketTypeName(PacketType packetType)
{
	switch (packetType)
	{
	case HELLO:
		return "HELLO";
	case INTEREST:
		return "INTEREST";
	case DATA:
		return "DATA";
    case DIGEST:
        return "DIGEST";
    case InterestUnknownContentProvider:
        return "InterestUnknownContentProvider";
    case InterestKnownContentProvider:
        return "InterestKnownContentProvider";
	}
	return "";
}

PktHeader::PktHeader ()
{
}

PktHeader::~PktHeader ()
{
}

TypeId
PktHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::PktHeader")
        .SetParent<Header> ()
        .AddConstructor<PktHeader> ()
        ;
        return tid;
}

TypeId
PktHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
PktHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU32 (m_source.Get ());
    start.WriteHtonU32 (m_destination.Get ());
    
    start.WriteHtonU32 ((uint32_t)m_packetType);
    start.WriteHtonU64 ((uintptr_t)m_socialTieTable);
    start.WriteHtonU32 (m_socialTieTableSize);
    
    start.WriteHtonU64 ((uintptr_t)m_higherCentralityList);
    start.WriteHtonU32 (m_higherCentralityListSize);
    
    start.WriteHtonU64 ((uintptr_t)m_contentArray);
    start.WriteHtonU32 (m_contentArraySize);
    
    start.WriteHtonU32 (m_interestBroadcastId);
    start.WriteHtonU32 (m_requestedContent);
    
    start.WriteHtonU32 (m_requesterId.Get ());
    start.WriteHtonU32 (m_contentProviderId.Get ());

    start.WriteHtonU64 (m_timestamp);
    start.WriteHtonU32 (m_sequenceNumber);
    
}

uint32_t
PktHeader::Deserialize (Buffer::Iterator start)
{
    Buffer::Iterator  i = start;

    m_source.Set (i.ReadNtohU32 ());
    m_destination.Set (i.ReadNtohU32 ());
    m_packetType = (PacketType)(i.ReadNtohU32());
    m_socialTieTable = (uint64_t *)(i.ReadNtohU64());
    m_socialTieTableSize = i.ReadNtohU32 ();
    m_higherCentralityList = (uint64_t *)(i.ReadNtohU64 ());
    m_higherCentralityListSize = i.ReadNtohU32 ();
    m_contentArray = (PatrolContent *)(i.ReadNtohU64 ());
    m_contentArraySize = i.ReadNtohU32 ();
    m_interestBroadcastId = i.ReadNtohU32 ();
    m_requestedContent = i.ReadNtohU32 ();
    m_requesterId.Set (i.ReadNtohU32 ());
    m_contentProviderId.Set (i.ReadNtohU32 ());
    m_timestamp = i.ReadNtohU64 ();
    m_sequenceNumber = i.ReadNtohU32 ();

    return GetSerializedSize ();
}

uint32_t
PktHeader::GetSerializedSize (void) const
{
    // Multiply by 13 for # of variables serialized
    return (19 * (sizeof (uint32_t)));
}

void
PktHeader::Print (std::ostream &os) const
{
    os << "Src = " << m_source << "Dest = " << m_destination << std::endl;
}

void
PktHeader::SetSource (Ipv4Address src)
{
    m_source = src;
}

void 
PktHeader::SetDestination (Ipv4Address dest)
{
    m_destination = dest;
}

Ipv4Address
PktHeader::GetSource (void) const
{
    return m_source;
}

Ipv4Address
PktHeader::GetDestination (void ) const
{
    return m_destination;
}

PacketType
PktHeader::GetPacketType() const
{
	return m_packetType;
}

void
PktHeader::SetPacketType(PacketType packetType)
{
	m_packetType = packetType;
}

uint64_t *
PktHeader::GetSocialTieTable() const
{
    void setRequestedContent(uint32_t *requestedContent);
    return m_socialTieTable;
}

void
PktHeader::SetSocialTieTable(uint64_t *socialTieTable)
{
    m_socialTieTable = socialTieTable;
}

uint32_t
PktHeader::GetSocialTieTableSize() const
{
    return m_socialTieTableSize;
}

void
PktHeader::SetSocialTieTableSize(uint32_t socialTieTableSize)
{
    m_socialTieTableSize = socialTieTableSize;
}

uint32_t
PktHeader::GetHigherCentralityListSize() const
{
    return m_higherCentralityListSize;
}

void
PktHeader::SetHigherCentralityListSize(uint32_t higherCentralityListSize)
{
    m_higherCentralityListSize = higherCentralityListSize;
}
   	
uint64_t *
PktHeader::GetHigherCentralityList() const
{
    return m_higherCentralityList;
}

void
PktHeader::SetHigherCentralityList(uint64_t *higherCentralityList)
{
    m_higherCentralityList = higherCentralityList;
}

uint32_t
PktHeader::GetContentArraySize() const
{
    return m_contentArraySize;
}

void
PktHeader::SetContentArraySize(uint32_t contentArraySize)
{
    m_contentArraySize = contentArraySize;
}
   	
PatrolContent *
PktHeader::GetContentArray() const
{
    return m_contentArray;
}

void
PktHeader::SetContentArray(PatrolContent *contentArray)
{
    m_contentArray = contentArray;
}

//For interest packet
uint32_t
PktHeader::GetInterestBroadcastId() const
{
    return m_interestBroadcastId;
}

void
PktHeader::SetInterestBroadcastId(uint32_t interestBroadcastId)
{
    m_interestBroadcastId = interestBroadcastId;
}

uint32_t
PktHeader::GetRequestedContent() const
{
    return m_requestedContent;
}

void 
PktHeader::SetRequestedContent(uint32_t requestedContent)
{
    m_requestedContent = requestedContent;
}

Ipv4Address
PktHeader::GetRequesterId(void) const
{
    return m_requesterId;
}

void
PktHeader::SetRequesterId(Ipv4Address requesterId)
{
    m_requesterId = requesterId;
}

Ipv4Address
PktHeader::GetContentProviderId(void) const
{
    return m_contentProviderId;
}

void
PktHeader::SetContentProviderId(Ipv4Address contentProviderId)
{
    m_contentProviderId = contentProviderId;
}

double 
PktHeader::GetTimeStamp(void) const
{
    return m_timestamp;
}

void 
PktHeader::SetTimeStamp(double timestamp)
{
    m_timestamp = timestamp;
}

uint32_t 
PktHeader::GetSequenceNumber(void) const
{
    return m_sequenceNumber;
}
void 
PktHeader::SetSequenceNumber(uint32_t sequenceNumber)
{
    m_sequenceNumber = sequenceNumber;
}


}
