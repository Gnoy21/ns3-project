#ifndef MINYONG_APPLICATION_H
#define MINYONG_APPLICATION_H

#include "ns3/application.h"
#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/mobility-module.h"
#include "ns3/simulator.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "math.h"
#include "minyong-pkt-header.h"
#include <string>

using namespace std;

namespace ns3
{

struct PendingData
{
    Ipv4Address human;
    double positionX;
    double positionY;
    double velocity;
    double angle;
    double timestamp;
    uint64_t* route;
    uint32_t routeSize;
    uint32_t locatedCell;
    uint32_t humanId;
};

enum NodeType { BS, UAV, HUMAN };

class MinyongApplication : public Application
{
public:
    MinyongApplication ();
    virtual ~MinyongApplication ();
    
    void SetNodeType (NodeType nodeType);
    void SetPort (uint16_t port);
    void SetNeighborNode (Ipv4Address neighborNode);
    void SetLinkStateInfo (int* linkStateInfo);
    void SetId (uint32_t id);

    void ResetRoute (vector<uint32_t> uavInfo);

    int GetLocatedCell ();

    vector<PendingData> GetPendingData ();

protected:
    virtual void DoDispose ();

private:
    virtual void StartApplication ();
    virtual void StopApplication ();
    Ipv4Address GetNodeAddress ();

    void PrintInformation ();

    void SendCheck ();
    void SendResponse ();
    void SendData ();
    void SendPacket (MinyongPktHeader header);
    MinyongPktHeader* CreateCheckPacketHeader ();
    MinyongPktHeader* CreateResponsePacketHeader ();
    MinyongPktHeader* CreateDataPacketHeader (Ipv4Address human, uint32_t locatedCell, double angle, double previousX, double previousY, double velocity, uint32_t humanId);

    void HandleRead (Ptr<Socket> socket);
    void HandleCheck (MinyongPktHeader* header);
    void HandleResponse (MinyongPktHeader* header);
    void HandleData (MinyongPktHeader* header);

    void SetLocatedCell ();
    void SetAngle ();
    void SetVelocity ();

    void SetDijkstraArray (int i);

    int GetLinkNumber (int i, int j);

    void DijkstraAlgorithm ();
    int MinDistance (int dist[], bool visited[]);
    void PrintPath (uint32_t parent[], int j);
    void PrintSolution (int dist[], int n, uint32_t parent[]);

    Ptr<Socket> m_socket;
    uint16_t m_port;

    Ipv4Address m_neighborNode;
    int m_neighborId;

    MinyongPktHeader* m_header;

    int* m_linkStateInfo;

    vector<PendingData> m_pending_data;

    uint32_t m_id;

    NodeType m_nodeType;
    uint32_t m_locatedCell;
    double m_angle;
    double m_previousX;
    double m_previousY;
    double m_velocity;

    int m_dijkstraArray[25][25];

    Ipv4Address tmp_source;
    uint32_t tmp_locatedCell;
    uint32_t tmp_humanId;
    double tmp_angle;
    double tmp_positionX;
    double tmp_positionY;
    double tmp_velocity;

    double m_sendTime;
};

}

#endif