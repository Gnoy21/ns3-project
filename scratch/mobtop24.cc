#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/paparazzi-mobility-model.h"
#include "ns3/test-mobility-model.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/social-network.h"
#include "ns3/spectrum-helper.h"
#include "ns3/adhoc-aloha-noack-ideal-phy-helper.h"
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>
#include <ns3/spectrum-model-300kHz-300GHz-log.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/spectrum-analyzer.h>
#include <ns3/friis-spectrum-propagation-loss.h>

//XXX For anumation
#include "ns3/netanim-module.h"

using namespace std;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Nhap9");

class Topology
{
public:
    Topology ();
    bool Configure (int argc, char *argv[]);
    void Run ();
    void PrintNames ();

private:
    uint32_t nodeNum;
    double duration;
    uint32_t seed;
    uint32_t fileType;
    double mapSize;
    uint8_t fNodeNum;
    bool useInterfer;

    NodeContainer nodes;
    NodeContainer interfererNodes;
    NetDeviceContainer devices; 
    NetDeviceContainer interfererDevices;
    Ipv4InterfaceContainer interfaces;
    
private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
  static void generateTraffic(Ptr<Socket> socket, uint32_t pktSize, double pktCount, Time pktInterval);
  Ipv4Address GetIpv4Address(uint32_t index);
  Ptr<Socket> SetupPacketReceive (Ptr<Node> node);
  void ReceivePacket (Ptr<Socket> socket);
};

Topology::Topology (): 
    nodeNum (10),
    duration (600),
    seed (2),
    fileType(1),
    mapSize(1000),
    useInterfer(true)
{ 
}

bool 
Topology::Configure (int argc, char *argv[])
{
    CommandLine cmd;

    cmd.AddValue ("nodeNum", "Number of nodes.", nodeNum);
    cmd.AddValue ("duration", "Simulation time in sec.", duration);
    cmd.AddValue ("seed", "Seed", seed);
    cmd.AddValue ("mapSize", "Size of Map", mapSize);
    cmd.AddValue ("fileType", "Type of Routing.", fileType);
    cmd.AddValue ("useInterfer", "use Interfer Node", useInterfer);

    cmd.Parse (argc, argv);
    return true;
}

void
Topology::Run ()
{
    fNodeNum = nodeNum/10;
    CreateNodes ();
    CreateDevices ();
    InstallInternetStack ();
    InstallApplications ();

    std::cout << "Starting simulation for " << duration << " s.\n";

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (duration));

    AnimationInterface anim ("test.xml");
    Simulator::Run ();
    Simulator::Destroy ();
}

void
Topology::CreateNodes ()
{
    std::cout << "Creating " << nodeNum << " nodes.\n";
    nodes.Create (nodeNum);
    if(useInterfer)
    {
        interfererNodes.Create (5);
    }
}

void
Topology::CreateDevices ()
{
    std::string phyMode ("DsssRate1Mbps");
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    //Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));    

    SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default ();
    channelHelper.AddPropagationLoss (
        "ns3::RangePropagationLossModel",
        "MaxRange",
        DoubleValue(200.0)
    );

    SpectrumWifiPhyHelper phy;
    phy.SetChannel (channelHelper.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager (
        "ns3::ConstantRateWifiManager",
        "DataMode",StringValue (phyMode),
        "ControlMode",StringValue (phyMode)
    );

    WifiMacHelper mac;
    mac.SetType ("ns3::AdhocWifiMac");

    devices = wifi.Install (phy, mac, nodes);
    
    if(useInterfer)
    {
        double txPower = 0.1; // Watts
        uint32_t channelNumber = 1;
        WifiSpectrumValue5MhzFactory sf;
        Ptr<SpectrumValue> txPsd =  sf.CreateTxPowerSpectralDensity (txPower, channelNumber);

        const double k = 1.381e-23; //Boltzmann's constant
        const double T = 290; // temperature in Kelvin
        double noisePsdValue = k * T; // watts per hertz
        Ptr<SpectrumValue> noisePsd = sf.CreateConstant (noisePsdValue);

        AdhocAlohaNoackIdealPhyHelper deviceHelper;
        deviceHelper.SetChannel (channelHelper.Create ());
        deviceHelper.SetTxPowerSpectralDensity (txPsd);
        deviceHelper.SetNoisePowerSpectralDensity (noisePsd);
        deviceHelper.SetPhyAttribute ("Rate", DataRateValue (DataRate ("1Mbps")));
        interfererDevices = deviceHelper.Install(interfererNodes);

        PacketSocketHelper packetSocket;
        packetSocket.Install (interfererNodes);

        PacketSocketAddress socket0;
        socket0.SetSingleDevice (interfererDevices.Get (0)->GetIfIndex ());
        socket0.SetPhysicalAddress (interfererDevices.Get (4)->GetAddress ());
        socket0.SetProtocol (1);

        PacketSocketAddress socket1;
        socket1.SetSingleDevice (interfererDevices.Get (1)->GetIfIndex ());
        socket1.SetPhysicalAddress (interfererDevices.Get (4)->GetAddress ());
        socket1.SetProtocol (1);

        PacketSocketAddress socket2;
        socket2.SetSingleDevice (interfererDevices.Get (2)->GetIfIndex ());
        socket2.SetPhysicalAddress (interfererDevices.Get (4)->GetAddress ());
        socket2.SetProtocol (1);

        PacketSocketAddress socket3;
        socket3.SetSingleDevice (interfererDevices.Get (3)->GetIfIndex ());
        socket3.SetPhysicalAddress (interfererDevices.Get (4)->GetAddress ());
        socket3.SetProtocol (1);

        OnOffHelper onoff0 ("ns3::PacketSocketFactory", Address (socket0));
        onoff0.SetConstantRate (DataRate ("0.5Mbps"));
        onoff0.SetAttribute ("PacketSize", UintegerValue (125));

        OnOffHelper onoff1 ("ns3::PacketSocketFactory", Address (socket1));
        onoff1.SetConstantRate (DataRate ("0.5Mbps"));
        onoff1.SetAttribute ("PacketSize", UintegerValue (125));

        OnOffHelper onoff2 ("ns3::PacketSocketFactory", Address (socket2));
        onoff2.SetConstantRate (DataRate ("0.5Mbps"));
        onoff2.SetAttribute ("PacketSize", UintegerValue (125));

        OnOffHelper onoff3 ("ns3::PacketSocketFactory", Address (socket3));
        onoff3.SetConstantRate (DataRate ("0.5Mbps"));
        onoff3.SetAttribute ("PacketSize", UintegerValue (125));

        ApplicationContainer apps = onoff0.Install (interfererNodes.Get (0));
        apps.Add(onoff1.Install (interfererNodes.Get (1)));
        apps.Add(onoff2.Install (interfererNodes.Get (2)));
        apps.Add(onoff3.Install (interfererNodes.Get (3)));
        apps.Start (Seconds (0.1));
        apps.Stop (Seconds (duration));

        Ptr<Socket> recvSink = SetupPacketReceive (interfererNodes.Get (4));
    }

    RngSeedManager::SetSeed(seed);

    MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
			"X", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+to_string(mapSize)+"]"),
			"Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+to_string(mapSize)+"]"),
			"Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));

	mobility.SetMobilityModel ("ns3::PaparazziMobilityModel",
			"Bounds", BoxValue (Box (0, mapSize, 0, mapSize, 0, 0)),
			"Speed", DoubleValue (11)); // 40km/h
    
    if(fileType == 0)
    {
        mobility.Install (nodes);
    }
    else
    {
        int tmp = 0;
        for(uint32_t i = 0; i < nodeNum-fNodeNum; i++)
        {
            mobility.Install (nodes.Get(i));
            tmp = i;
        }
    
    
        MobilityHelper mobility1;
	    mobility1.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
		    	"X", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+to_string(mapSize)+"]"),
		    	"Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+to_string(mapSize)+"]"),
		    	"Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));

	    mobility1.SetMobilityModel ("ns3::TestMobilityModel",
		    	"Bounds", BoxValue (Box (0, mapSize, 0, mapSize, 0, 0)),
		    	"Speed", DoubleValue (33)); // 120km/h
	
        for(uint32_t i = tmp+1; i < nodeNum; i++)
        {
            mobility1.Install (nodes.Get(i));
        }


    }

    if(useInterfer)
    {
        MobilityHelper mobility2;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector (200.0, 200.0, 0.0));
        positionAlloc->Add (Vector (800.0, 200.0, 0.0));
        positionAlloc->Add (Vector (200.0, 800.0, 0.0));
        positionAlloc->Add (Vector (800.0, 800.0, 0.0));
        positionAlloc->Add (Vector (1500.0, 1500.0, 0.0));
        mobility2.SetPositionAllocator (positionAlloc);
        mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility2.Install (interfererNodes);
    }
}

void
Topology::ReceivePacket (Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    uint64_t bytes = 0;
    while ((packet = socket->Recv ()))
    {
        bytes += packet->GetSize ();
    }
  
    //std::cout << "SOCKET received " << bytes << " bytes" << std::endl;
}

Ptr<Socket>
Topology::SetupPacketReceive (Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  sink->Bind ();
  //sink->SetRecvCallback (MakeCallback (&Topology::ReceivePacket));
  return sink;
}

void
Topology::InstallInternetStack ()
{
    InternetStackHelper stack;
    stack.Install (nodes);
    // stack.Install (interfererNodes);
    
    Ipv4AddressHelper address;
    address.SetBase ("1.0.0.0", "255.0.0.0");
    interfaces = address.Assign (devices);

    /*
    if(useInterfer)
    {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        for (int i = 0; i < 4; i++)
        {
            Ptr<Socket> interferer = Socket::CreateSocket (interfererNodes.Get(i), tid);
            InetSocketAddress interferingAddr = InetSocketAddress (Ipv4Address ("255.255.255.255"), 49000);
            interferer->SetAllowBroadcast (true);
            interferer->Connect (interferingAddr);

            Simulator::ScheduleWithContext (interferer->GetNode()->GetId(), 
                                            Seconds(1), 
                                            &Topology::generateTraffic, 
                                            interferer, 1000, duration, Seconds(1.0));
        }
    }
    */
}

void
Topology::InstallApplications ()
{
    Ptr<SocialNetwork> app[nodeNum];
    for (uint32_t i =0; i < nodeNum; i++)
    {   
        app[i] = CreateObject<SocialNetwork> ();
        app[i]->Setup (9); //Set port
        nodes.Get (i)->AddApplication (app[i]);
        app[i]->SetStartTime (Seconds (0.5 + 0.0001*i));
        app[i]->SetStopTime (Seconds (duration));
    }
    
	// Use one node or multiple nodes VVV
    uint32_t requestors = nodeNum/10;

	srand (time(NULL));
    // Number of requesting applications
    uint32_t i = 0;

    uint32_t *selectedRequestors = new uint32_t[requestors];

    while (i < requestors)
    {
        uint32_t requNode = rand () % nodeNum;
        if(fileType == 1)
        {
            requNode = rand () % (nodeNum-fNodeNum);
        }
        bool j = false;

        for(uint32_t k = 0; k < requestors; k++)
        {
            if(selectedRequestors[k] == requNode)
            {
                j = true;
            }
        }

        if(j)
        {
            continue;
        }

        selectedRequestors[i] = requNode;
        
        uint32_t contNode = rand () % 100;
        
        cout << "Requestor " << GetIpv4Address(requNode) << endl;
        if (requNode == 0 || requNode == 1)
        {
            app[requNode+1]->RequestContent(contNode);
            app[requNode+2]->SetContent(contNode);
        }
        else
        {
            app[requNode-1]->RequestContent(contNode);
            app[requNode-2]->SetContent(contNode);
        }
        i++;
    }

    delete[] selectedRequestors;
}

void
Topology::PrintNames ()
{
    for (uint32_t i=0; i< nodeNum; i++)
        std::cout << Names::FindName (nodes.Get(i)) << std::endl;
}

Ipv4Address
Topology::GetIpv4Address(uint32_t index)
{
	return nodes.Get(index)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
}

void
Topology::generateTraffic(Ptr<Socket> socket, uint32_t pktSize, double pktCount, Time pktInterval)
{
    if (pktCount > 0)
    {
        socket->Send (Create<Packet> (pktSize));
        Simulator::Schedule (pktInterval, &Topology::generateTraffic, socket, pktSize, pktCount - 1, pktInterval);
    }
    else
    {
        socket->Close ();
    }
}

int
main (int argc, char *argv[])
{
    Topology test;
    if (! test.Configure (argc, argv))
        NS_FATAL_ERROR ("Configuration failed. Aborted.");

    test.Run ();

    return 0;
}
