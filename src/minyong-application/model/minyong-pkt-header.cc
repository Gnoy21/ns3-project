#include "ns3/log.h"
#include "minyong-pkt-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MinyongPktHeader");
NS_OBJECT_ENSURE_REGISTERED (MinyongPktHeader);

MinyongPktHeader::MinyongPktHeader ()
{
    m_edge = new uint64_t[40];
    m_edgeSize = 0;
}

MinyongPktHeader::~MinyongPktHeader ()
{
}

TypeId
MinyongPktHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("ns3::MinyongPktHeader")
    .SetParent<Header> ()
    .AddConstructor<MinyongPktHeader> ();

    return tid;
}

TypeId
MinyongPktHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}

void
MinyongPktHeader::Print (std::ostream &os) const
{
}

uint32_t
MinyongPktHeader::GetSerializedSize () const
{
    return (19 * (sizeof (uint32_t)));
}

void
MinyongPktHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU32 (m_source.Get ());
    start.WriteHtonU32 (m_human.Get ());

    start.WriteHtonU64 ((uintptr_t)m_edge);
    start.WriteHtonU32 (m_edgeSize);

    start.WriteHtonU32 (m_id);
    start.WriteHtonU32 (m_humanId);

    start.WriteHtonU32 ((uint32_t)m_packetType);
    start.WriteHtonU32 (m_locatedCell);

    start.WriteHtonU64 (m_angle * 100000.0);
    start.WriteHtonU64 (m_positionX * 100000.0);
    start.WriteHtonU64 (m_positionY * 100000.0);
    start.WriteHtonU64 (m_velocity * 100000.0);

    start.WriteHtonU64 (m_timestamp * 100000.0);
}

uint32_t
MinyongPktHeader::Deserialize (Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    m_source.Set (i.ReadNtohU32 ());
    m_human.Set (i.ReadNtohU32 ());

    m_edge = (uint64_t*)(i.ReadNtohU64 ());
    m_edgeSize = i.ReadNtohU32 ();

    m_id = i.ReadNtohU32 ();
    m_humanId = i.ReadNtohU32 ();

    m_packetType = (PacketType)(i.ReadNtohU32 ());
    m_locatedCell = i.ReadNtohU32 ();

    m_angle = i.ReadNtohU64 () / 100000.0;
    m_positionX = i.ReadNtohU64 () / 100000.0;
    m_positionY = i.ReadNtohU64 () / 100000.0;
    m_velocity = i.ReadNtohU64 () / 100000.0;

    m_timestamp = i.ReadNtohU64 () / 100000.0;

    return GetSerializedSize ();
}

PacketType
MinyongPktHeader::GetPacketType () const
{
    return m_packetType;
}

void
MinyongPktHeader::SetPacketType (PacketType packetType)
{
    m_packetType = packetType;
}

Ipv4Address
MinyongPktHeader::GetSource () const
{
    return m_source;
}

void
MinyongPktHeader::SetSource (Ipv4Address source)
{
    m_source = source;
}

Ipv4Address
MinyongPktHeader::GetHuman () const
{
    return m_human;
}

void
MinyongPktHeader::SetHuman (Ipv4Address human)
{
    m_human = human;
}

uint32_t
MinyongPktHeader::GetId () const
{
    return m_id;
}

void
MinyongPktHeader::SetId (uint32_t id)
{
    m_id = id;
}

double 
MinyongPktHeader::GetPositionX () const
{
    return m_positionX;
}

void
MinyongPktHeader::SetPositionX (double positionX)
{
    m_positionX = positionX;
}

double 
MinyongPktHeader::GetPositionY () const
{
    return m_positionY;
}

void
MinyongPktHeader::SetPositionY (double positionY)
{
    m_positionY = positionY;
}

double
MinyongPktHeader::GetVelocity () const
{
    return m_velocity;
}

void
MinyongPktHeader::SetVelocity (double velocity)
{
    m_velocity = velocity;
}

double
MinyongPktHeader::GetAngle () const
{
    return m_angle;
}

void
MinyongPktHeader::SetAngle (double angle)
{
    m_angle = angle;
}

uint32_t
MinyongPktHeader::GetLocatedCell () const
{
    return m_locatedCell;
}

void
MinyongPktHeader::SetLocatedCell (uint32_t locatedCell)
{
    m_locatedCell = locatedCell;
}

double
MinyongPktHeader::GetTimeStamp () const
{
    return m_timestamp;
}

void
MinyongPktHeader::SetTimeStamp (double timestamp)
{
    m_timestamp = timestamp;
}

uint64_t*
MinyongPktHeader::GetEdge () const
{
    return m_edge;
}

void
MinyongPktHeader::SetEdge (uint64_t* edge)
{
    m_edge = edge;
}

uint32_t
MinyongPktHeader::GetEdgeSize () const
{
    return m_edgeSize;
}

void
MinyongPktHeader::SetEdgeSize (uint32_t edgeSize)
{
    m_edgeSize = edgeSize;
}

uint32_t 
MinyongPktHeader::GetHumanId () const
{
    return m_humanId;
}

void 
MinyongPktHeader::SetHumanId (uint32_t humanId)
{
    m_humanId = humanId;
}