#include "minyong-application.h"

#define INF 1000

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MinyongApplication");
NS_OBJECT_ENSURE_REGISTERED (MinyongApplication);

MinyongApplication::MinyongApplication ()
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;
    m_linkStateInfo = new int[40];

    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 25; j++)
        {
            m_dijkstraArray[i][j] = 0;
        }
    }
}

MinyongApplication::~MinyongApplication ()
{
    NS_LOG_FUNCTION (this);

    m_socket = 0;
}

void
MinyongApplication::SetPort (uint16_t port)
{
    m_port = port;
}

void
MinyongApplication::SetNodeType (NodeType nodeType)
{
    m_nodeType = nodeType;
}

void 
MinyongApplication::SetNeighborNode (Ipv4Address neighborNode)
{
    m_neighborNode = neighborNode;
}

void
MinyongApplication::SetId (uint32_t id)
{
    m_id = id;
}

void
MinyongApplication::ResetRoute (vector<uint32_t> uavInfo)
{
    int id = 2;
    for (vector<uint32_t>::iterator iter = uavInfo.begin (); iter != uavInfo.end (); iter++)
    {
        if (*iter == 0)
        {
            if (!(((id + 1) % 10 == 7) || ((id + 1) % 10 == 2)))
            {
                m_dijkstraArray[id-2][id-1] = 0;
                m_dijkstraArray[id-1][id-2] = 0;
            }
            
            if (!(((id - 1) % 10 == 1) || ((id - 1) % 10 == 6)))
            {
                m_dijkstraArray[id-2][id-3] = 0;
                m_dijkstraArray[id-3][id-2] = 0;
            }

            if (id - 5 > 2)
            {
                m_dijkstraArray[id-2][id-7] = 0;
                m_dijkstraArray[id-7][id-2] = 0;
            }

            if (id + 5 < 27)
            {
                m_dijkstraArray[id-2][id+3] = 0;
                m_dijkstraArray[id+3][id-2] = 0;
            }
        }
        id++;
    }

    //cout << "Inside ResetRout" << endl;

    for (int i = 0; i < 25; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                if (m_dijkstraArray[i][j] != INF)
                {
                    //cout << m_dijkstraArray[i][j] << "\t";
                }
                else
                {
                    //cout << "0\t";
                }
            }
            //cout << "" << endl;
        }

    DijkstraAlgorithm ();
}

void
MinyongApplication::SetLinkStateInfo (int* linkStateInfo)
{
    m_linkStateInfo = linkStateInfo;
}

vector<PendingData>
MinyongApplication::GetPendingData ()
{
    vector<PendingData> data;

    vector<PendingData>::iterator iter;
    for (iter = m_pending_data.begin (); iter != m_pending_data.end (); iter++)
    {
        PendingData tmpData;
        tmpData.human = iter->human;
        tmpData.angle = iter->angle;
        tmpData.positionX = iter->positionX;
        tmpData.positionY = iter->positionY;
        tmpData.velocity = iter->velocity;
        tmpData.route = iter->route;
        tmpData.routeSize = iter->routeSize;
        tmpData.humanId = iter->humanId;
        tmpData.timestamp = iter->timestamp;
        NS_LOG_UNCOND ("test : " << tmpData.humanId);
        data.push_back (tmpData);
    }

    m_pending_data.clear ();

    return data;
}

void
MinyongApplication::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
}

void
MinyongApplication::StartApplication ()
{
    SetLocatedCell ();

    vector<uint32_t> test;

    for (int i = 0; i < 25; i++)
    {
        test.push_back (1);
    }

    if (m_nodeType == HUMAN)
    {
        SetAngle ();
        SetVelocity ();
    }
    
    if (m_socket == 0)
    {
        m_socket = Socket::CreateSocket (GetNode (), TypeId::LookupByName ("ns3::UdpSocketFactory"));
        m_socket->SetAllowBroadcast (true);
        m_socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_port));
        m_socket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast (), m_port));
    }

    m_socket->SetRecvCallback (MakeCallback (&MinyongApplication::HandleRead, this));

    if (m_nodeType == UAV)
    {
        //SendCheck ();
        /*
        for (int i = 0; i < 40; i++)
        {
            cout << m_linkStateInfo[i] << " ";
        }*/

        //cout << endl;
        
        for (int i = 0; i < 25; i++)
        {
            SetDijkstraArray (i);
        }

        for (int i = 0; i < 25; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                if (m_dijkstraArray[i][j] != INF)
                {
                    //cout << m_dijkstraArray[i][j] << "\t";
                }
                else
                {
                    //cout << "0\t";
                }
            }
            //cout << "" << endl;
        }

        //DijkstraAlgorithm ();

        ResetRoute (test);
    }

    if (m_nodeType == HUMAN)
    {
        //Simulator::Schedule(Seconds(60), &MinyongApplication::SendResponse, this);
        Simulator::Schedule(Seconds(60), &MinyongApplication::PrintInformation, this);
    }
}

void
MinyongApplication::PrintInformation ()
{
    NS_LOG_INFO (GetNodeAddress () << " " << m_previousX << " " << m_previousY << " " << m_angle << " " << m_velocity << " " << (int (Simulator::Now ().GetSeconds ()) / 10) * 10);

    Simulator::Schedule(Seconds(60), &MinyongApplication::PrintInformation, this);
}

void
MinyongApplication::StopApplication ()
{
    NS_LOG_FUNCTION (this);

    if (m_socket != 0)
    {
        m_socket->Close ();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
        m_socket = 0;
    }
}

Ipv4Address
MinyongApplication::GetNodeAddress ()
{
    Ptr<Node> node = GetNode ();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address thisIpv4Address = ipv4->GetAddress (1,0).GetLocal ();
    return thisIpv4Address;
}

void
MinyongApplication::SendCheck ()
{
    MinyongPktHeader* header = CreateCheckPacketHeader ();
    SendPacket (*header);

    Simulator::Schedule(Seconds(5), &MinyongApplication::SendCheck, this);
}

void
MinyongApplication::SendResponse ()
{
    MinyongPktHeader* header = CreateResponsePacketHeader ();
    SendPacket (*header);

    Simulator::Schedule(Seconds(60), &MinyongApplication::SendResponse, this);
}

void
MinyongApplication::SendData ()
{
    MinyongPktHeader* header = CreateDataPacketHeader (
        tmp_source,
        tmp_locatedCell,
        tmp_angle,
        tmp_positionX,
        tmp_positionY,
        tmp_velocity,
        tmp_humanId
    );

    m_socket->Connect (InetSocketAddress (m_neighborNode, m_port));
    SendPacket (*header);
}

void
MinyongApplication::SendPacket(MinyongPktHeader header)
{
    Ptr<Packet> p = Create<Packet> ();
    p->AddHeader(header);

    if (m_socket != 0)
    {
        m_socket->Send (p);
    }
}


MinyongPktHeader*
MinyongApplication::CreateCheckPacketHeader ()
{
    MinyongPktHeader *header = new MinyongPktHeader ();
    header->SetPacketType (CHECK);
    header->SetSource (GetNodeAddress ());
    header->SetLocatedCell (m_locatedCell);

    return header;
}

MinyongPktHeader*
MinyongApplication::CreateResponsePacketHeader ()
{
    MinyongPktHeader *header = new MinyongPktHeader ();
    header->SetSource (GetNodeAddress ());
    header->SetPacketType (RESPONSE);
    header->SetLocatedCell (m_locatedCell);
    header->SetAngle (m_angle);
    header->SetPositionX (m_previousX);
    header->SetHumanId (m_id);
    header->SetPositionY (m_previousY);
    header->SetVelocity (m_velocity);

    return header;
}

MinyongPktHeader*
MinyongApplication::CreateDataPacketHeader (Ipv4Address human, uint32_t locatedCell, double angle, double previousX, double previousY, double velocity, uint32_t humanId)
{
    MinyongPktHeader *header = new MinyongPktHeader ();
    header->SetSource (GetNodeAddress ());
    header->SetHuman (human);
    header->SetPacketType (DATA);
    header->SetLocatedCell (locatedCell);
    header->SetId (m_id);
    header->SetAngle (angle);
    header->SetPositionX (previousX);
    header->SetPositionY (previousY);
    header->SetVelocity (velocity);
    header->SetHumanId (humanId);
    header->SetTimeStamp (Simulator::Now ().GetSeconds ());
    
    return header;
}

void
MinyongApplication::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;
    
    while ((packet = socket->RecvFrom (from)))
    {
        MinyongPktHeader *header = new MinyongPktHeader ();
        packet->PeekHeader (*header);

        PacketType packetType = header->GetPacketType ();
        switch (packetType)
        {
        case CHECK:
            if (m_nodeType == HUMAN || m_nodeType == UAV)
            {
                HandleCheck (header);
            }
            break;

        case RESPONSE:
            if (m_nodeType == UAV)
            {
                HandleResponse (header);
            }
            break;

        case DATA:
            HandleData (header);
            break;
        
        default:
            return;
        }
    }
}

void
MinyongApplication::HandleCheck (MinyongPktHeader* header)
{
    if (m_nodeType == HUMAN)
    {
        if (header->GetLocatedCell () == m_locatedCell)
        {
            m_neighborNode = header->GetSource ();
        }
    }
    
}

void
MinyongApplication::HandleResponse (MinyongPktHeader *header)
{
    if (header->GetLocatedCell () == m_locatedCell)
    {
        tmp_source = header->GetSource ();
        tmp_locatedCell = header->GetLocatedCell ();
        tmp_angle = header->GetAngle ();
        tmp_positionX = header->GetPositionX ();
        tmp_positionY = header->GetPositionY ();
        tmp_velocity = header->GetVelocity ();
        tmp_humanId = header->GetHumanId ();

        if (m_neighborNode != Ipv4Address ("1.0.0.255"))
        {
            SendData ();
        }
    }
}

void
MinyongApplication::HandleData (MinyongPktHeader *header)
{
    if (m_nodeType == BS)
    {
        PendingData data;
        data.human = header->GetHuman ();
        data.angle = header->GetAngle ();
        data.positionX = header->GetPositionX ();
        data.positionY = header->GetPositionY ();
        data.velocity = header->GetVelocity ();
        data.timestamp = header->GetTimeStamp ();
        data.locatedCell = header->GetLocatedCell ();
        data.routeSize = header->GetEdgeSize ();
        data.route = header->GetEdge ();
        data.humanId = header->GetHumanId ();
        m_pending_data.push_back (data);
    }
    else if (m_nodeType == UAV)
    {
        uint64_t* edge = header->GetEdge ();
        uint32_t edgeSize = header->GetEdgeSize ();

        uint32_t id = header->GetId ();
        uint32_t biggerId;
        uint32_t smallerId;

        if (m_id > id)
        {
            biggerId = m_id;
            smallerId = id;
        }
        else
        {
            biggerId = id;
            smallerId = m_id;
        }

        if (biggerId - smallerId == 1)
        {
            if (biggerId < 7)
            {
                edge[edgeSize] = biggerId - 2;
            }
            else if (biggerId < 12)
            {
                edge[edgeSize] = biggerId - 3;
            }
            else if (biggerId < 17)
            {
                edge[edgeSize] = biggerId - 4;
            }
            else if (biggerId < 22)
            {
                edge[edgeSize] = biggerId - 5;
            }
            else if (biggerId < 27)
            {
                edge[edgeSize] = biggerId - 6;
            }
            edgeSize++;
            header->SetEdge (edge);
            header->SetEdgeSize (edgeSize);
        }
        else if (biggerId - smallerId == 5)
        {
            edge[edgeSize] = (biggerId * 3) - ((biggerId - 7) * 2);
            edgeSize++;
            header->SetEdge (edge);
            header->SetEdgeSize (edgeSize);
        }

        if (m_neighborNode != Ipv4Address ("1.0.0.255"))
        {
            header->SetId (m_id);
            m_socket->Connect (InetSocketAddress (m_neighborNode, m_port));
            SendPacket(*header);
            m_socket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast (), m_port));
        }
        
    }
}

void
MinyongApplication::SetLocatedCell ()
{
    Ptr<MobilityModel> mob = GetNode ()->GetObject<MobilityModel> ();
    m_previousX = mob->GetPosition ().x;
    m_previousY = mob->GetPosition ().y;

    int x = int (m_previousX / 100);
    int y = int (m_previousY / 100) + 1;

    m_locatedCell = (5 * x) + y;

    Simulator::Schedule(Seconds(2), &MinyongApplication::SetLocatedCell, this);
}

void
MinyongApplication::SetAngle ()
{
    Ptr<MobilityModel> mob = GetNode ()->GetObject<MobilityModel> ();
    double x = mob->GetPosition ().x;
    double y = mob->GetPosition ().y;

    double radian = atan2 (x - m_previousX, y - m_previousY);
    if (radian < 0)
    {
        m_angle = fabs (radian * 180 / M_PI) + 180;
    }
    else
    {
        m_angle = radian * 180 / M_PI;
    }

    Simulator::Schedule(Seconds(1), &MinyongApplication::SetAngle, this);
}

void
MinyongApplication::SetVelocity ()
{
    Ptr<MobilityModel> mob = GetNode ()->GetObject<MobilityModel> ();
    double x = mob->GetPosition ().x;
    double y = mob->GetPosition ().y;
    double distance = sqrt(pow(x - m_previousX, 2) + pow(y - m_previousY, 2));
    m_velocity = distance;

    Simulator::Schedule(Seconds(1), &MinyongApplication::SetVelocity, this);
}

void
MinyongApplication::SetDijkstraArray (int i)
{
    int id = i + 2;

    if (!(((id + 1) % 10 == 7) || ((id + 1) % 10 == 2)))
    {
        m_dijkstraArray[id-2][id-1] = 1;
        m_dijkstraArray[id-1][id-2] = 1;
    }
    
    if (!(((id - 1) % 10 == 1) || ((id - 1) % 10 == 6)))
    {
        m_dijkstraArray[id-2][id-3] = 1;
        m_dijkstraArray[id-3][id-2] = 1;
    }

    if (id - 5 > 2)
    {
        m_dijkstraArray[id-2][id-7] = 1;
        m_dijkstraArray[id-7][id-2] = 1;
    }

    if (id + 5 < 27)
    {
        m_dijkstraArray[id-2][id+3] = 1;
        m_dijkstraArray[id+3][id-2] = 1;
    }
    
}

int
MinyongApplication::GetLinkNumber (int i, int j)
{
    uint32_t biggerId;
    uint32_t smallerId;

    if (i > j)
    {
        biggerId = i;
        smallerId = j;
    }
    else
    {
        biggerId = j;
        smallerId = i;
    }

    if (biggerId - smallerId == 1)
    {
        if (biggerId < 7)
        {
            return biggerId - 2;
        }
        else if (biggerId < 12)
        {
            return biggerId - 3;
        }
        else if (biggerId < 17)
        {
            return biggerId - 4;
        }
        else if (biggerId < 22)
        {
            return biggerId - 5;
        }
        else if (biggerId < 27)
        {
            return biggerId - 6;
        }
    }
    else if (biggerId - smallerId == 5)
    {
        return (biggerId * 3) - ((biggerId - 7) * 2);
    }

    return 0;
}

void
MinyongApplication::DijkstraAlgorithm ()
{
    int dist[25];
    bool sptSet[25];
    uint32_t parent[25];
    int src = 0;

    for (int i = 0; i < 25; i++)
    {
        dist[i] = INF;
        parent[i] = 375;
        sptSet[i] = false;
    }

    parent[1] = 0;
    parent[5] = 0;

    dist[src] = 0;

    for (int i = 0; i < 24; i++)
    {
        int u = MinDistance (dist, sptSet);
        sptSet[u] = true;

        for (int j = 0; j < 25; j++)
        {
            if (!sptSet[j] && m_dijkstraArray[u][j] && dist[u] + m_dijkstraArray[u][j] < dist[j])
            {
                parent[j] = u;
                dist[j] = dist[u] + m_dijkstraArray[u][j];
            }
        }
    }

    if (parent[m_id-2] == 375)
    {
        string tmpIp = "1.0.0." + to_string(255);
        
        const char* c = tmpIp.c_str ();
        m_neighborNode.Set(c);
    }
    else
    {
        m_neighborId = parent[m_id-2] + 2;

        string tmpIp = "1.0.0." + to_string(m_neighborId);
        
        const char* c = tmpIp.c_str ();
        m_neighborNode.Set(c);
    }

    if (m_id == 2)
    {
        m_neighborId = 1;
        string tmpIp = "1.0.0." + to_string(m_neighborId);
        
        const char* c = tmpIp.c_str ();
        m_neighborNode.Set(c);
    }

    //PrintSolution (dist, 25, parent);
}

int
MinyongApplication::MinDistance (int dist[], bool visited[])
{
    int min = INF;
    int min_index = 0;

    for (int v = 0; v < 25; v++)
    {
        if (visited[v] == false && dist[v] <= min)
        {
            min = dist[v];
            min_index = v;
        }
    }

    return min_index;
}

void
MinyongApplication::PrintSolution (int dist[], int n, uint32_t parent[])
{
    int src = 0;
    cout << "Vertex\t Distance\tPath";

    for (int i = 0; i < n; i++)
    {
        printf ("\n%d -> %d \t\t %d\t\t%d", src, i, dist[i], src);
        PrintPath (parent, i);
    }
}

void
MinyongApplication::PrintPath (uint32_t parent[], int j)
{
    if (parent[j] == 375)
    {
        return;
    }

    PrintPath (parent, parent[j]);
    cout << j << " ";
}

}