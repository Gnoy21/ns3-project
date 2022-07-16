#ifndef MINYONG_PKT_HEADER_H
#define MINYONG_PKT_HEADER_H


#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-address.h"

using namespace std;

namespace ns3
{

enum PacketType { CHECK, RESPONSE, DATA };

class MinyongPktHeader : public Header
{
public:
    MinyongPktHeader ();
    virtual ~MinyongPktHeader ();

    static TypeId GetTypeId ();
    virtual TypeId GetInstanceTypeId () const;
    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize () const;

    uint64_t* GetEdge () const;
    void SetEdge (uint64_t* edge);

    uint32_t GetEdgeSize () const;
    void SetEdgeSize (uint32_t edgeSize);

    PacketType GetPacketType () const;
    void SetPacketType (PacketType packetType);

    Ipv4Address GetSource () const;
    void SetSource (Ipv4Address source);

    Ipv4Address GetHuman () const;
    void SetHuman (Ipv4Address human);

    double GetPositionX () const;
    void SetPositionX (double positionX);

    double GetPositionY () const;
    void SetPositionY (double positionY);

    double GetVelocity () const;
    void SetVelocity (double velocity);

    double GetAngle () const;
    void SetAngle (double angle);

    uint32_t GetLocatedCell () const;
    void SetLocatedCell (uint32_t locatedCell);

    double GetTimeStamp () const;
    void SetTimeStamp (double timestamp);

    uint32_t GetId () const;
    void SetId (uint32_t id);

    uint32_t GetHumanId () const;
    void SetHumanId (uint32_t humanId);

private:
    Ipv4Address m_source;
    Ipv4Address m_human;

    uint64_t* m_edge;
    uint32_t m_edgeSize;

    PacketType m_packetType;
    
    double m_positionX;
    double m_positionY;
    double m_velocity;
    double m_angle;

    double m_timestamp;

    uint32_t m_locatedCell;

    uint16_t m_headerSize;

    uint32_t m_id;
    uint32_t m_humanId;

};

}

#endif