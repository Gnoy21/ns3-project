
#ifndef PKT_HEADER_H
#define PKT_HEADER_H

#include "ns3/header.h"
#include "ns3/address-utils.h"
#include "ns3/ipv4-address.h"
#include "patrol-content.h"

using namespace std;

namespace ns3
{
//tuan
enum PacketType { HELLO, INTEREST, DATA, DIGEST, InterestUnknownContentProvider, InterestKnownContentProvider };

string GetPacketTypeName(PacketType packetType);

class PktHeader : public Header
{
public:
    PktHeader ();
    virtual ~PktHeader ();
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Print (std::ostream &os) const;
    void SetSource (Ipv4Address src);
    void SetDestination (Ipv4Address dest);
    Ipv4Address GetSource (void) const;
    Ipv4Address GetDestination (void ) const;

    PacketType GetPacketType() const;
    void SetPacketType(PacketType packetType);
    uint64_t *GetSocialTieTable() const;
    void SetSocialTieTable(uint64_t *socialTieTable);
    uint32_t GetSocialTieTableSize() const;
    void SetSocialTieTableSize(uint32_t socialTieTableSize);
   	
    uint32_t GetHigherCentralityListSize() const;
    void SetHigherCentralityListSize(uint32_t higherCentralityListSize);
   	
    uint64_t *GetHigherCentralityList() const;
    void SetHigherCentralityList(uint64_t *higherCentralityList);
   	
    uint32_t GetContentArraySize() const;
    void SetContentArraySize(uint32_t contentArraySize);
   	
   	PatrolContent *GetContentArray() const;
   	void SetContentArray(PatrolContent *contentArray);
   	
   	//For interest packet
    uint32_t GetInterestBroadcastId() const;
    void SetInterestBroadcastId(uint32_t interestBroadcastId);

   	uint32_t GetRequestedContent() const;
   	void SetRequestedContent(uint32_t requestedContent);
   	
   	Ipv4Address GetRequesterId(void) const;
   	void SetRequesterId(Ipv4Address requesterId);
   	
   	Ipv4Address GetContentProviderId(void) const;
   	void SetContentProviderId(Ipv4Address contentProviderId);

    double GetTimeStamp(void) const;
    void SetTimeStamp(double timestamp);

    uint32_t GetSequenceNumber(void) const;
    void SetSequenceNumber(uint32_t sequenceNumber);

private:
    // These 2 fields uniquely identify an Interest packet
    Ipv4Address m_source;
    uint32_t m_interestBroadcastId;
    uint32_t m_requestedContent;
    
    Ipv4Address m_destination;
    uint16_t m_headerSize;
    uint64_t *m_socialTieTable;
    uint32_t m_socialTieTableSize;
    uint32_t m_higherCentralityListSize;
    uint64_t *m_higherCentralityList;
    uint32_t m_contentArraySize;
    PatrolContent *m_contentArray;
    Ipv4Address m_requesterId;
    Ipv4Address m_contentProviderId;
    
    double m_timestamp;
    uint32_t m_sequenceNumber;
    
    PacketType m_packetType;

};

}	// namespace ns3

#endif /* PKT_HEADER_H */
